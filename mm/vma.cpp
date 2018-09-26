/* SPDX-License-Identifier: MIT */

/*
 * mm/vma.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/mm/vma.h>
#include <infos/mm/mm.h>
#include <infos/kernel/kernel.h>
#include <infos/util/string.h>

using namespace infos::mm;
using namespace infos::kernel;
using namespace infos::util;

VMA::VMA()
{
	auto pgd = allocate_phys(0);
	assert(pgd);
	
	_pgt_phys_base = sys.mm().pgalloc().pgd_to_pa(pgd);
	_pgt_virt_base = sys.mm().pgalloc().pgd_to_vpa(pgd);
}

VMA::~VMA()
{
	// TODO: Release allocations
	
}

// This is a hack.  In fact, this whole file is a hack because it's
// not architecture independent.
extern uint64_t *__template_pml4;

void VMA::install_default_kernel_mapping()
{
	assert(__template_pml4);
	
	uint64_t *pgt = (uint64_t *)_pgt_virt_base;
	
	pgt[0x1ff] = __template_pml4[0x1ff];
	pgt[0x100] = __template_pml4[0x100];
}

#define BITS(val, start, end) ((((uint64_t)val) >> start) & (((1 << (end - start + 1)) - 1)))
			
typedef uint16_t table_idx_t;

static inline void va_table_indicies(virt_addr_t va, table_idx_t& pm, table_idx_t& pdp, table_idx_t& pd, table_idx_t& pt)
{
	pm = BITS(va, 39, 47);
	pdp = BITS(va, 30, 38);
	pd = BITS(va, 21, 29);
	pt = BITS(va, 12, 20);
}

struct MMUTableEntry {
	enum MMUTableEntryFlags {
		// PTE
		PRESENT		= 0,
		WRITABLE	= 1,
		ALLOW_USER	= 2,
		WRITE_THROUGH	= 3,
		CACHE_DISABLED	= 4,
		ACCESSED	= 5,
		DIRTY		= 6,
		HUGE		= 7,
		GLOBAL		= 8,
	};
	
	union {
		uint64_t bits;
	};
	
	inline phys_addr_t base_address() const {
		return bits & ~0xfff;
	}

	inline void base_address(phys_addr_t addr) {
		bits &= 0xfff;
		bits |= addr & ~0xfff;
	}

	inline uint16_t flags() const {
		return bits & 0xfff;
	}

	inline void flags(uint16_t flags) {
		bits &= ~0xfff;
		bits |= flags & 0xfff;
	}
	
	inline bool get_flag(MMUTableEntryFlags idx) const { return !!(flags() & (1 << idx)); }
	inline void set_flag(MMUTableEntryFlags idx, bool v) { 
		if (!v) {
			flags(flags() & ~(1 << (uint16_t)idx)); 
		} else {
			flags(flags() | (1 << (uint16_t)idx));
		}
	}
	
	inline bool present() const { return get_flag(PRESENT); }
	inline void present(bool v) { set_flag(PRESENT, v); }
	
	inline bool writable() const { return get_flag(WRITABLE); }
	inline void writable(bool v) { set_flag(WRITABLE, v); }

	inline bool user() const { return get_flag(ALLOW_USER); }
	inline void user(bool v) { set_flag(ALLOW_USER, v); }

	inline bool huge() const { return get_flag(HUGE); }
	inline void huge(bool v) { set_flag(HUGE, v); }
} __packed;

struct PML4TableEntry : MMUTableEntry {
} __packed;

struct PDPTableEntry : MMUTableEntry {
} __packed;

struct PDTableEntry : MMUTableEntry {
} __packed;

struct PTTableEntry : MMUTableEntry {
} __packed;

void VMA::insert_mapping(virt_addr_t va, phys_addr_t pa, MappingFlags::MappingFlags flags)
{
	table_idx_t pml4_idx, pdp_idx, pd_idx, pt_idx;
	va_table_indicies(va, pml4_idx, pdp_idx, pd_idx, pt_idx);
	
	PML4TableEntry *pml4 = &((PML4TableEntry *)_pgt_virt_base)[pml4_idx];
	
	if (pml4->base_address() == 0) {
		auto pdp = allocate_phys(0);
		assert(pdp);
		
		pml4->base_address(sys.mm().pgalloc().pgd_to_pa(pdp));
		pml4->present(true);
		pml4->writable(true);
		pml4->user(true);
	}
	
	PDPTableEntry *pdp = &((PDPTableEntry *)pa_to_vpa(pml4->base_address()))[pdp_idx];
	
	if (pdp->base_address() == 0) {
		auto pd = allocate_phys(0);
		assert(pd);
		
		pdp->base_address(sys.mm().pgalloc().pgd_to_pa(pd));
		pdp->present(true);
		pdp->writable(true);
		pdp->user(true);
	}
	
	PDTableEntry *pd = &((PDTableEntry *)pa_to_vpa(pdp->base_address()))[pd_idx];
	
	if (pd->base_address() == 0) {
		auto pt = allocate_phys(0);
		assert(pt);
		
		pd->base_address(sys.mm().pgalloc().pgd_to_pa(pt));
		pd->present(true);
		pd->writable(true);
		pd->user(true);
	}
	
	PTTableEntry *pt = &((PTTableEntry *)pa_to_vpa(pd->base_address()))[pt_idx];
	
	pt->base_address(pa);
	
	if (flags & MappingFlags::Present) pt->present(true);
	if (flags & MappingFlags::Writable) pt->writable(true);
	if (flags & MappingFlags::User) pt->user(true);
	
	mm_log.messagef(LogLevel::DEBUG, "vma: mapping va=%p -> pa=%p", va, pa);
}

PageDescriptor *VMA::allocate_phys(int order)
{
	auto pgd = sys.mm().pgalloc().alloc_pages(order);
	if (!pgd) return NULL;
	
	PageAllocation pa;
	pa.descriptor_base = pgd;
	pa.allocation_order = order;
	
	_page_allocations.append(pa);
	pnzero((void *)sys.mm().pgalloc().pgd_to_vpa(pa.descriptor_base), 1 << order);
	
	return pgd;
}

bool VMA::allocate_virt_any(int nr_pages)
{
	return false;
}

bool VMA::allocate_virt(virt_addr_t va, int nr_pages)
{
	if (nr_pages == 0) return false;
	
	int order = __log2ceil(nr_pages);
	
	const PageDescriptor *pgd = allocate_phys(order);
	if (!pgd) {
		return false;
	}
	
	virt_addr_t vbase = va;
	phys_addr_t pbase = sys.mm().pgalloc().pgd_to_pa(pgd);
	for (unsigned int i = 0; i < (1u << order); i++) {
		insert_mapping(vbase, pbase, MappingFlags::Present | MappingFlags::User | MappingFlags::Writable);
		
		vbase += 0x1000;
		pbase += 0x1000;
	}
	
	return true;
}

bool VMA::is_mapped(virt_addr_t va)
{
	phys_addr_t pa;
	return get_mapping(va, pa);
}

bool VMA::get_mapping(virt_addr_t va, phys_addr_t& pa)
{
	if (!_pgt_virt_base) return false;
	
	table_idx_t pml4_idx, pdp_idx, pd_idx, pt_idx;
	va_table_indicies(va, pml4_idx, pdp_idx, pd_idx, pt_idx);
	
	PML4TableEntry *pml4 = &((PML4TableEntry *)_pgt_virt_base)[pml4_idx];
	
	if (!pml4->present()) {
		return false;
	}
	
	PDPTableEntry *pdp = &((PDPTableEntry *)pa_to_vpa(pml4->base_address()))[pdp_idx];
	
	if (!pdp->present()) {
		return false;
	}
	
	PDTableEntry *pd = &((PDTableEntry *)pa_to_vpa(pdp->base_address()))[pd_idx];
	
	if (!pd->present()) {
		return false;
	}
	
	PTTableEntry *pt = &((PTTableEntry *)pa_to_vpa(pd->base_address()))[pt_idx];
	
	if (!pt->present()) {
		return false;
	}
	
	pa = pt->base_address() | __page_offset(va);
	return true;
}


bool VMA::copy_to(virt_addr_t dest_va, const void* src, size_t size)
{
	phys_addr_t pa;
	if (!get_mapping(dest_va, pa))
		return false;
	
	void *dest = (void *)pa_to_vpa(pa);
	
	//mm_log.messagef(LogLevel::DEBUG, "vma: copy-to dst=va=%p:pa=%p:vpa=%p src=%p size=%p", dest_va, pa, dest, src, size);
	memcpy(dest, src, size);
	
	return true;
}

void VMA::dump()
{
	PML4TableEntry *te = (PML4TableEntry *)_pgt_virt_base;
	for (unsigned int i = 0; i < 0x200; i++) {
		if (!te[i].present()) continue;
		if (te[i].huge()) {
			uintptr_t va = (uint64_t)i << 36;
			mm_log.messagef(LogLevel::DEBUG, "VMA: MAP VA=%p -> PA=%p", va, te[i].base_address());
		} else {
			dump_pdp(i, pa_to_vpa(te[i].base_address()));
		}
	}
}

void VMA::dump_pdp(int pml4, virt_addr_t pdp_va)
{
	PDPTableEntry *te = (PDPTableEntry *)pdp_va;
	for (unsigned int i = 0; i < 0x200; i++) {
		if (!te[i].present()) continue;
		if (te[i].huge()) {
			uintptr_t va = (uint64_t)pml4 << 36 | (uint64_t)i << 28;
			mm_log.messagef(LogLevel::DEBUG, "VMA: MAP VA=%p -> PA=%p", va, te[i].base_address());	
		} else {
			dump_pd(pml4, i, pa_to_vpa(te[i].base_address()));
		}
	}
}

void VMA::dump_pd(int pml4, int pdp, virt_addr_t pd_va)
{
	PDTableEntry *te = (PDTableEntry *)pd_va;
	for (unsigned int i = 0; i < 0x200; i++) {
		if (!te[i].present()) continue;
		if (te[i].huge()) {
			uintptr_t va = (uint64_t)pml4 << 36 | (uint64_t)pdp << 28 | (uint64_t)i << 20;
			mm_log.messagef(LogLevel::DEBUG, "VMA: MAP VA=%p -> PA=%p", va, te[i].base_address());
		} else {
			dump_pt(pml4, pdp, i, pa_to_vpa(te[i].base_address()));
		}
	}
}

void VMA::dump_pt(int pml4, int pdp, int pd, virt_addr_t pt_va)
{
	PTTableEntry *te = (PTTableEntry *)pt_va;
	for (unsigned int i = 0; i < 0x200; i++) {
		if (!te[i].present()) continue;
		
		uintptr_t va = (uint64_t)pml4 << 36 | (uint64_t)pdp << 28 | (uint64_t)pd << 20 | (uint64_t)i << 12;
		mm_log.messagef(LogLevel::DEBUG, "VMA: MAP VA=%p -> PA=%p", va, te[i].base_address());
	}
}
