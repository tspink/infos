/* SPDX-License-Identifier: MIT */

/*
 * mm/page-allocator.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/mm/page-allocator.h>
#include <infos/mm/mm.h>
#include <infos/util/string.h>
#include <infos/util/lock.h>
#include <infos/util/cmdline.h>

extern char _IMAGE_START, _IMAGE_END;
extern char _STACK_START, _STACK_END;
extern char _HEAP_START;

using namespace infos::mm;
using namespace infos::kernel;
using namespace infos::util;

ComponentLog infos::mm::pgalloc_log(syslog, "pgalloc");

static bool do_self_test;

RegisterCmdLineArgument(PageAllocDebug, "pgalloc.debug")
{
	if (strncmp(value, "1", 1) == 0)
	{
		pgalloc_log.enable();
	}
	else
	{
		pgalloc_log.disable();
	}
}

RegisterCmdLineArgument(PageAllocSelfTest, "pgalloc.self-test")
{
	if (strncmp(value, "1", 2) == 0)
	{
		do_self_test = true;
	}
	else
	{
		do_self_test = false;
	}
}

PageAllocator::PageAllocator(MemoryManager &mm) : Allocator(mm), _page_descriptors(NULL)
{
}

bool PageAllocator::setup_page_descriptors()
{
	// Store the total number of pages in the system.
	_nr_pages = owner()._last_pfn + 1;

	// Calculate the total size of the page descriptor array.
	uint64_t pd_size = _nr_pages * sizeof(PageDescriptor);

	// Use the start of the heap area as the basis for the page descriptor array.
	_page_descriptors = (PageDescriptor *)&_HEAP_START;
	mm_log.messagef(LogLevel::DEBUG, "Allocating %lu page descriptors (%lu kB)", _nr_pages, KB(pd_size));

	// Make sure the page descriptors will fit in this region of memory
	const PhysicalMemoryBlock *pmb = owner().lookup_phys_block(kva_to_pa((virt_addr_t)_page_descriptors));
	if (!pmb)
	{
		return false;
	}

	// TODO: Actually check this assertion holds, using the size of the physical memory block.

	// Initialise the page descriptors, by zeroing them all.
	bzero(_page_descriptors, _nr_pages * sizeof(PageDescriptor));

	return true;
}

bool PageAllocator::init()
{
	// The page allocation algorithm must be specified before initialising
	// the page allocator.
	if (!_allocator_algorithm)
	{
		mm_log.message(LogLevel::ERROR, "Page allocator algorithm not registered");
		return false;
	}

	// Prepare the page descriptor array
	if (!setup_page_descriptors())
	{
		return false;
	}

	// Initialise the page allocator algorithm
	mm_log.messagef(LogLevel::INFO, "Initialising allocator algorithm '%s'", _allocator_algorithm->name());
	if (!_allocator_algorithm->init(_page_descriptors, _nr_pages))
	{
		mm_log.message(LogLevel::ERROR, "Allocator failed to initialise");
		return false;
	}

	uint64_t nr_present_pages, nr_free_pages;

	// Loop through available physical memory blocks, and update the corresponding page descriptors.
	nr_present_pages = 0;
	for (unsigned int i = 0; i < owner()._nr_phys_mem_blocks; i++)
	{
		const PhysicalMemoryBlock &pmb = owner()._phys_mem_blocks[i];

		if (pmb.type == MemoryType::NORMAL)
		{
			nr_present_pages += pmb.nr_pages;
			for (pfn_t pfn = pmb.base_pfn; pfn < (pmb.base_pfn + pmb.nr_pages); pfn++)
			{
				_page_descriptors[pfn].type = PageDescriptorType::AVAILABLE;
			}

			_allocator_algorithm->insert_page_range(&_page_descriptors[pmb.base_pfn], pmb.nr_pages);
		}
	}

	nr_free_pages = nr_present_pages;

	// Reserve page zero.  We can do without it.
	nr_free_pages -= reserve_page_range(0, 1);

	// Reserve initial page table pages
	nr_free_pages -= reserve_page_range(1, 6);

	// Now, reserve the kernel's pages

	// Reserve the range of pages corresponding to the kernel image
	pfn_t image_start_pfn = pa_to_pfn((phys_addr_t)&_IMAGE_START); // _IMAGE_START is a PA
	pfn_t image_last_pfn = pa_to_pfn((phys_addr_t)&_IMAGE_END);	   // _IMAGE_END is a PA

	nr_free_pages -= reserve_page_range(image_start_pfn, image_last_pfn - image_start_pfn + 1);

	// Reserve the range of pages corresponding to the kernel stack
	pfn_t stack_start_pfn = pa_to_pfn(kva_to_pa((virt_addr_t)&_STACK_START)); // _STACK_START is a VA
	pfn_t stack_last_pfn = pa_to_pfn(kva_to_pa((virt_addr_t)&_STACK_END));	  // _STACK_END is a VA
	nr_free_pages -= reserve_page_range(stack_start_pfn, stack_last_pfn - stack_start_pfn + 1);

	// Reserve the range of pages corresponding to the kernel heap, which is
	// actually just the page descriptors.
	pfn_t heap_start_pfn = pa_to_pfn(kva_to_pa((virt_addr_t)&_HEAP_START)); // _HEAP_START is a VA
	nr_free_pages -= reserve_page_range(heap_start_pfn, ((_nr_pages * sizeof(PageDescriptor)) >> 12) + 1);

	mm_log.messagef(LogLevel::INFO, "Page Allocator: total=%lu, present=%lu, free=%lu (%u MB)", _nr_pages, nr_present_pages, nr_free_pages, MB(nr_free_pages << 12));

	// Now, initialise the page allocation algorithm.

	if (do_self_test)
	{
		if (!self_test())
		{
			mm_log.message(LogLevel::FATAL, "Allocator self-test failed!");
			arch_abort();
		}
	}

	return true;
}

uint64_t PageAllocator::reserve_page_range(pfn_t start, uint64_t nr_pages)
{
	for (pfn_t pfn = start; pfn < start + nr_pages; pfn++)
	{
		_page_descriptors[pfn].type = PageDescriptorType::RESERVED;
	}

	_allocator_algorithm->remove_page_range(&_page_descriptors[start], _nr_pages);

	return nr_pages;
}

/**
 * Allocates 2^order contiguous pages
 * @param order The power of two of the number of pages to allocate
 * @return Returns a pointer to an array of page descriptors representing the new allocation, or NULL if allocation
 * failed.
 */
PageDescriptor *PageAllocator::alloc_pages(int order)
{
	// Call into the algorithm to actually allocate the pages.
	if (!_allocator_algorithm)
		return NULL;

	UniqueLock<Mutex> l(_mtx);
	PageDescriptor *pgd = _allocator_algorithm->allocate_pages(order);

	// Double check that all the pages are marked as available, and
	// mark them as allocated.
	for (unsigned int i = 0; i < (1u << order); i++)
	{
		assert(pgd[i].type == PageDescriptorType::AVAILABLE);
		pgd[i].type = PageDescriptorType::ALLOCATED;
	}

	pgalloc_log.messagef(LogLevel::DEBUG, "alloc: order=%d, pgd=%p (%lx)", order, pgd, pgd_to_pa(pgd));
	return pgd;
}

/**
 * Frees 2^order contiguous pages
 * @param pgd A pointer to an array of 2^order contiguous pages
 * @param order The power of two of the number of pages to free
 */
void PageAllocator::free_pages(PageDescriptor *pgd, int order)
{
	// Call into the algorithm to actually free the pages.
	if (_allocator_algorithm)
	{
		UniqueLock<Mutex> l(_mtx);
		_allocator_algorithm->free_pages(pgd, order);

		// Double-check that all the pages were allocated, and mark them as available.
		for (unsigned int i = 0; i < (1u << order); i++)
		{
			assert(pgd[i].type == PageDescriptorType::ALLOCATED);
			pgd[i].type = PageDescriptorType::AVAILABLE;
		}

		pgalloc_log.messagef(LogLevel::DEBUG, "free: order=%d, pgd=%p (%lx)", order, pgd, pgd_to_pa(pgd));
	}
}

const PageDescriptor *PageAllocator::alloc_zero_page()
{
	const PageDescriptor *pgd = alloc_page();
	if (!pgd)
		return NULL;

	void *ptr = (void *)pgd_to_vpa(pgd);
	pzero(ptr);

	return pgd;
}

bool PageAllocator::self_test()
{
	assert(_allocator_algorithm);

	mm_log.messagef(LogLevel::IMPORTANT, "PAGE ALLOCATOR SELF TEST - BEGIN");
	mm_log.messagef(LogLevel::IMPORTANT, "------------------------");

	mm_log.messagef(LogLevel::INFO, "* INITIAL STATE");
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "------------------------");
	mm_log.messagef(LogLevel::INFO, "(1) ALLOCATING ONE PAGE");
	auto p = _allocator_algorithm->allocate_pages(0);
	if (!p)
	{
		mm_log.messagef(LogLevel::ERROR, "Allocator did not allocate page");
		return false;
	}

	mm_log.messagef(LogLevel::INFO, "ALLOCATED PFN: %p", pgd_to_pfn(p));
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "------------------------");
	mm_log.messagef(LogLevel::INFO, "(2) FREEING ONE PAGE");
	_allocator_algorithm->free_pages(p, 0);

	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "------------------------");
	mm_log.messagef(LogLevel::INFO, "(3) ALLOCATING TWO CONTIGUOUS PAGES");
	p = _allocator_algorithm->allocate_pages(1);
	mm_log.messagef(LogLevel::INFO, "ALLOCATED PFN: %p", pgd_to_pfn(p));

	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "------------------------");
	mm_log.messagef(LogLevel::INFO, "(4) FREEING TWO CONTIGUOUS PAGES");
	_allocator_algorithm->free_pages(p, 1);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "------------------------");
	mm_log.messagef(LogLevel::INFO, "(5) OVERLAPPING ALLOCATIONS");
	auto p0 = _allocator_algorithm->allocate_pages(0);
	auto p1 = _allocator_algorithm->allocate_pages(1);
	_allocator_algorithm->free_pages(p0, 0);
	auto p2 = _allocator_algorithm->allocate_pages(0);
	_allocator_algorithm->free_pages(p1, 1);
	_allocator_algorithm->free_pages(p2, 0);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "------------------------");
	mm_log.messagef(LogLevel::INFO, "(6) MULTIPLE ALLOCATIONS, RANDOM ORDER FREE");
	auto p00 = _allocator_algorithm->allocate_pages(0);
	auto p01 = _allocator_algorithm->allocate_pages(0);
	auto p02 = _allocator_algorithm->allocate_pages(0);
	auto p03 = _allocator_algorithm->allocate_pages(2);
	auto p04 = _allocator_algorithm->allocate_pages(0);
	auto p05 = _allocator_algorithm->allocate_pages(0);
	auto p06 = _allocator_algorithm->allocate_pages(0);
	auto p07 = _allocator_algorithm->allocate_pages(0);
	mm_log.messagef(LogLevel::INFO, "* AFTER ALLOCATION");
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "  FREE %p", pgd_to_pfn(p05));
	_allocator_algorithm->free_pages(p05, 0);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "  FREE %p", pgd_to_pfn(p03));
	_allocator_algorithm->free_pages(p03, 2);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "  FREE %p", pgd_to_pfn(p06));
	_allocator_algorithm->free_pages(p06, 0);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "  FREE %p", pgd_to_pfn(p07));
	_allocator_algorithm->free_pages(p07, 0);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "  FREE %p", pgd_to_pfn(p02));
	_allocator_algorithm->free_pages(p02, 0);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "  FREE %p", pgd_to_pfn(p01));
	_allocator_algorithm->free_pages(p01, 0);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "  FREE %p", pgd_to_pfn(p00));
	_allocator_algorithm->free_pages(p00, 0);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "  FREE %p", pgd_to_pfn(p04));
	_allocator_algorithm->free_pages(p04, 0);

	mm_log.messagef(LogLevel::INFO, "* AFTER RANDOM ORDER FREEING");
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "------------------------");
	mm_log.messagef(LogLevel::INFO, "(7) RESERVING PAGE 0x14e000 and 0x14f000");
	//bool page_reserved = _allocator_algorithm->reserve_page(pfn_to_pgd(0x14e));
	//assert(page_reserved);
	//page_reserved = _allocator_algorithm->reserve_page(pfn_to_pgd(0x14f));
	//assert(page_reserved);

	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "------------------------");
	mm_log.messagef(LogLevel::INFO, "(8) FREEING RESERVED PAGE 0x14f000");
	_allocator_algorithm->free_pages(pfn_to_pgd(0x14f), 0);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "------------------------");
	mm_log.messagef(LogLevel::INFO, "(9) FREEING RESERVED PAGE 0x14e000");
	_allocator_algorithm->free_pages(pfn_to_pgd(0x14e), 0);
	_allocator_algorithm->dump_state();

	mm_log.messagef(LogLevel::INFO, "------------------------");
	mm_log.messagef(LogLevel::INFO, "PAGE ALLOCATOR SELF TEST - COMPLETE");

	return true;
}

void PageAllocatorAlgorithm::dump_state() const
{
	pgalloc_log.messagef(LogLevel::WARNING, "dump_state() not implemented in allocation algorithm");
}
