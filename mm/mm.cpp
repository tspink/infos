/* SPDX-License-Identifier: MIT */

/*
 * mm/mm.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/mm/mm.h>
#include <infos/mm/page-allocator.h>
#include <infos/mm/object-allocator.h>
#include <infos/kernel/log.h>
#include <infos/util/cmdline.h>
#include <infos/util/string.h>

using namespace infos::mm;
using namespace infos::kernel;
using namespace infos::util;

ComponentLog infos::mm::mm_log(syslog, "mm");

static char pgalloc_algorithm[32];

RegisterCmdLineArgument(PageAllocAlgorithm, "pgalloc.algorithm") {
	strncpy(pgalloc_algorithm, value, sizeof(pgalloc_algorithm)-1);
}

MemoryManager::MemoryManager(Kernel& owner) 
	: Subsystem(owner), 
		_last_pfn(0), 
		_page_alloc(*this), 
		_obj_alloc(*this)
{

}

void MemoryManager::add_physical_memory(phys_addr_t addr, unsigned int nr_pages, MemoryType::MemoryType type)
{
	// Don't add any physical memory blocks that contain no pages.
	if (nr_pages == 0) {
		return;
	}
	
	// We only support a fixed number of physical memory blocks, to simply
	// allocation of the memory block structure at this point.
	if (_nr_phys_mem_blocks >= ARRAY_SIZE(_phys_mem_blocks)) {
		mm_log.messagef(LogLevel::WARNING, "Too many physical memory blocks");
		return;
	}
	
	mm_log.messagef(LogLevel::INFO, "PMB: %010lx--%010lx (%d kB) %s", 
			addr, 
			addr + (nr_pages * _page_size), 
			KB(nr_pages * _page_size),
			(type == MemoryType::UNUSABLE) ? "(unusable)" : "");
	
	// Fill in a new physical memory block structure
	_phys_mem_blocks[_nr_phys_mem_blocks].base_address = addr;
	_phys_mem_blocks[_nr_phys_mem_blocks].base_pfn = addr >> 12;
	_phys_mem_blocks[_nr_phys_mem_blocks].nr_pages = nr_pages;
	_phys_mem_blocks[_nr_phys_mem_blocks].type = type;
	_nr_phys_mem_blocks++;
	
	// Only normal memory counts towards the last PFN
	if (type == MemoryType::NORMAL) {
		pfn_t last_pfn = ((addr + (nr_pages * _page_size)) >> 12) - 1;
		if (last_pfn > _last_pfn) {
			_last_pfn = last_pfn;
		}
	}
}

bool MemoryManager::test_page_allocator_order(int order)
{
	mm_log.messagef(LogLevel::DEBUG, "%d page allocation", 1 << order);
	
	const PageDescriptor *pages8 = _page_alloc.alloc_pages(order);
	
	mm_log.message(LogLevel::DEBUG, "---");
	for (int i = 0; i < (1<<order); i++) {
		mm_log.messagef(LogLevel::DEBUG, "[%d] pfn=%lx addr=%p", 
				i,
				_page_alloc.pgd_to_pfn(&pages8[i]),
				_page_alloc.pgd_to_vpa(&pages8[i]));
	}
	
	return true;
}

bool MemoryManager::test_page_allocator()
{
	/*test_page_allocator_order(3);
	test_page_allocator_order(3);
	test_page_allocator_order(9);
	test_page_allocator_order(9);
	test_page_allocator_order(9);
	
	for (int i = 0; i < 100000000; i++) asm volatile("nop");*/
	
	/*test_page_allocator_order(0);
	test_page_allocator_order(2);
	test_page_allocator_order(4);
	test_page_allocator_order(0);
	test_page_allocator_order(1);
	test_page_allocator_order(0);
	test_page_allocator_order(1);
	test_page_allocator_order(0);
	test_page_allocator_order(1);
	test_page_allocator_order(5);
	test_page_allocator_order(2);
	test_page_allocator_order(0);
	test_page_allocator_order(1);*/
	//test_page_allocator_order(9);
	return true;
}


bool MemoryManager::initialise_allocators()
{
	// Print out some allocation statistics
	mm_log.messagef(LogLevel::INFO, "Total physical memory: %lx (%d MB)",
			((_last_pfn + 1) * _page_size),
			((_last_pfn + 1) * _page_size) / 1048576);
	
	mm_log.messagef(LogLevel::INFO, "Last PFN: %lx (%lx)", _last_pfn, _last_pfn * _page_size);
	
	// Set-up the page allocation algorithm
	PageAllocatorAlgorithm *algo = acquire_page_allocator_algorithm();
	if (!algo) {
		mm_log.messagef(LogLevel::ERROR, "No allocator algorithm available");
		return false;
	}
	
	syslog.messagef(LogLevel::IMPORTANT, "*** USING PAGE ALLOCATION ALGORITHM: %s", algo->name());
	
	// Install the discovered algorithm.
	_page_alloc.algorithm(*algo);
	
	// Initialise the page allocator.
	if (!_page_alloc.init()) {
		mm_log.message(LogLevel::ERROR, "Page allocator failed to initialise");
		return false;
	}
	
	if (!test_page_allocator()) {
		mm_log.message(LogLevel::ERROR, "Page allocator self-test failed");
		return false;
	}

	// Initialise the object allocator.
	if (!_obj_alloc.init()) {
		mm_log.message(LogLevel::ERROR, "Object allocator failed to initialise");
		return false;
	}
	
	return true;
}

const PhysicalMemoryBlock *MemoryManager::lookup_phys_block(phys_addr_t addr)
{
	// Iterate over each physical memory block, and return the descriptor
	// if the specified address is within its range.
	for (unsigned int i = 0; i < _nr_phys_mem_blocks; i++) {
		PhysicalMemoryBlock& block = _phys_mem_blocks[i];
		
		if (addr >= block.base_address && addr < (block.base_address + (block.nr_pages * _page_size))) {
			return &block;
		}
	}
	
	return NULL;
}

extern char _PGALLOC_PTR_START, _PGALLOC_PTR_END;

PageAllocatorAlgorithm* MemoryManager::acquire_page_allocator_algorithm()
{
	if (strlen(pgalloc_algorithm) == 0) {
		mm_log.messagef(LogLevel::ERROR, "Page allocation algorithm not chosen on command-line");
		return NULL;
	}
	
	PageAllocatorAlgorithm *candidate = NULL;
	PageAllocatorAlgorithm **pgallocators = (PageAllocatorAlgorithm **)&_PGALLOC_PTR_START;
	
	mm_log.messagef(LogLevel::DEBUG, "Searching for '%s' algorithm...", pgalloc_algorithm);
	while (pgallocators < (PageAllocatorAlgorithm **)&_PGALLOC_PTR_END) {
		if (strncmp((*pgallocators)->name(), pgalloc_algorithm, sizeof(pgalloc_algorithm)-1) == 0) {
			candidate = *pgallocators;
		}
		
		pgallocators++;
	}
			
	return candidate;
}
