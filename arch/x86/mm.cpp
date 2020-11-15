/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/mm.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/init.h>
#include <arch/x86/multiboot.h>
#include <arch/x86/cpuid.h>
#include <arch/x86/irq.h>
#include <arch/x86/context.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/thread.h>
#include <infos/kernel/process.h>
#include <infos/kernel/log.h>
#include <infos/mm/mm.h>
#include <infos/mm/page-allocator.h>
#include <infos/util/string.h>
#include <arch/arch.h>
#include <arch/x86/x86-arch.h>

using namespace infos::arch::x86;
using namespace infos::kernel;
using namespace infos::mm;
using namespace infos::util;

extern "C" infos::kernel::Thread *current_thread;

/**
 * Page fault handler
 * @param irq The IRQ object associated with this exception.
 * @param priv Private data associated with this exception.
 */
static void handle_page_fault(const IRQ *irq, void *priv)
{
	// Retrieve the fault_address from the cr2 control register.
	uint64_t fault_address;
	asm volatile("mov %%cr2, %0" : "=r"(fault_address));
	
	if (current_thread == NULL) {
		// If there is no current_thread, then this page fault happened REALLY
		// early.  We must abort.
		syslog.messagef(LogLevel::FATAL, "*** PAGE FAULT @ vaddr=%p", fault_address);
		
		arch_abort();
	}

	// If there is a current thread, abort it.
	syslog.messagef(LogLevel::WARNING, "*** PAGE FAULT @ vaddr=%p rip=%p", fault_address, current_thread->context().native_context->rip);
	
	// TODO: support passing page-faults into threads.
	current_thread->owner().terminate(-1);
}

// Ugly hack
uint64_t *__template_pml4;
const PageDescriptor *pages;

/**
 * Usurps the initial page tables, and introduces new ones for the
 * higher mapping.
 * @return Returns true if the page tables were successfully reinitialised,
 * or false otherwise.
 */
static bool reinitialise_pgt()
{
	// Allocate sixteen pages to mess around with.
	pages = sys.mm().pgalloc().alloc_pages(4);
	if (!pages) {
		x86_log.message(LogLevel::ERROR, "The page allocator has not worked!");
		return false;
	}
	
	// The PML4 will be the first page.
	uint64_t *pml4 = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[0]);
	
	// Zero all the pages.
	bzero(pml4, (1 << 4) * 0x1000);
	
	x86_log.messagef(LogLevel::DEBUG, "Kernel page tables @ %p", pml4);

	// Populate some local variables that will make working with the various
	// page tables at the various levels a bit easier.
	uint64_t *pdp0 = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[1]);
	uint64_t *pdp1 = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[2]);

	uint64_t *pdp0_pd0 = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[3]);
	uint64_t *pdp0_pd1 = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[4]);

	uint64_t *pdp1_pd0 = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[5]);
	uint64_t *pdp1_pd1 = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[6]);
	uint64_t *pdp1_pd2 = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[7]);
	uint64_t *pdp1_pd3 = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[8]);
	
	/*
	 * PML4 -> pdp{0,1}
	 * 
	 * PDP0 -> pd{0,1}
	 * PDP1 -> pd{0,1,2,3}
	 */
		
	// Fill in the PML4 and PDPs.
	pml4[0x1ff] = (kva_to_pa((virt_addr_t)pdp0) & ~0xfff) | 0x003;
	pml4[0x100] = (kva_to_pa((virt_addr_t)pdp1) & ~0xfff) | 0x003;
	
	pdp0[0x1fe] = (kva_to_pa((virt_addr_t)pdp0_pd0) & ~0xfff) | 0x003;
	pdp0[0x1ff] = (kva_to_pa((virt_addr_t)pdp0_pd1) & ~0xfff) | 0x003;
	
	pdp1[0x000] = (kva_to_pa((virt_addr_t)pdp1_pd0) & ~0xfff) | 0x003;
	pdp1[0x001] = (kva_to_pa((virt_addr_t)pdp1_pd1) & ~0xfff) | 0x003;
	pdp1[0x002] = (kva_to_pa((virt_addr_t)pdp1_pd2) & ~0xfff) | 0x003;
	pdp1[0x003] = (kva_to_pa((virt_addr_t)pdp1_pd3) & ~0xfff) | 0x003;

	// Now, fill in the KERNEL PDs
	uintptr_t addr = 0;
	for (unsigned int i = 0; i < 0x200; i++) {
		pdp0_pd0[i] = (addr & ~0xfff) | 0x083;
		pdp0_pd1[i] = ((addr + 0x40000000) & ~0xfff) | 0x083;
		addr += 0x200000;
	}

	// Now, fill in the PHYSMEM PDs for a 4G mapping
	addr = 0;
	for (unsigned int i = 0; i < 0x200; i++) {
		pdp1_pd0[i] = (addr & ~0xfff) | 0x083;
		addr += 0x200000;
	}
	
	for (unsigned int i = 0; i < 0x200; i++) {
		pdp1_pd1[i] = (addr & ~0xfff) | 0x083;
		addr += 0x200000;
	}
	
	for (unsigned int i = 0; i < 0x200; i++) {
		pdp1_pd2[i] = (addr & ~0xfff) | 0x083;
		addr += 0x200000;
	}
	
	for (unsigned int i = 0; i < 0x200; i++) {
		pdp1_pd3[i] = (addr & ~0xfff) | 0x083;
		addr += 0x200000;
	}

	// Zero page mapping for multicore startup
    uint64_t *pdpz = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[9]);
    uint64_t *pdz = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[10]);

    pml4[0] = (kva_to_pa((virt_addr_t)pdpz) & ~0xfff) | 0x003;
    pdpz[0] = (kva_to_pa((virt_addr_t)pdz) & ~0xfff) | 0x003;

    uint64_t base_addr = 0;
    for (int i = 0; i < 0x200; i++) {
        pdz[i] = (base_addr & ~0xfffull) | 0x083;
        base_addr += 0x200000;
    }
	
	// Welp, here we go.  Reload the page tables
	asm volatile ("mov %0, %%cr3" :: "r"(kva_to_pa((virt_addr_t)pml4)));
	
	// Hack in the template
	__template_pml4 = (uint64_t *)sys.mm().pgalloc().pgd_to_vpa(&pages[0]);
	
	return true;
}

void infos::arch::x86::mm_remove_multicore_mapping() {
    uint64_t *pml4 = (uint64_t *)sys.mm().pgalloc().pgd_to_kva(&pages[0]);
    pml4[0] = 0;
}

/**
 * Initialises the memory management subsystem.
 * @return Returns true if initialisation was successful, false otherwise.
 */
bool infos::arch::x86::mm_init()
{
	for (unsigned int i = 0; i < multiboot_info_structure->mmap_length / sizeof(struct multiboot_mmap_entry); i++) {
		struct multiboot_mmap_entry *entry = (struct multiboot_mmap_entry *)pa_to_kva(multiboot_info_structure->mmap_addr + (sizeof(struct multiboot_mmap_entry) * i));

		if ((entry->len & ~0xfff) != 0) {
			MemoryType::MemoryType type = MemoryType::NORMAL;
			
			switch (entry->type) {				
			case MULTIBOOT_MEMORY_AVAILABLE:
				type = MemoryType::NORMAL;
				break;
			case MULTIBOOT_MEMORY_RESERVED:
				type = MemoryType::UNUSABLE;
				break;
			default:
				x86_log.messagef(LogLevel::WARNING, "Skipping memory block with unknown type");
				continue;
			}
			
			sys.mm().add_physical_memory(entry->addr, (entry->len >> 12), type);
		}
	}
	
	if (!sys.mm().initialise_allocators())
		return false;
	
	return reinitialise_pgt();
}

bool infos::arch::x86::mm_pf_init()
{
	// Register the page-fault handler
	return x86arch.irq_manager().install_exception_handler(IRQ_PAGE_FAULT, handle_page_fault, NULL);
}