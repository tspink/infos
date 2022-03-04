/* SPDX-License-Identifier: MIT */

/*
 * include/mm/page-allocator.h
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/mm/allocator.h>
#include <infos/kernel/log.h>
#include <infos/util/lock.h>

namespace infos
{
	namespace mm
	{
		namespace PageDescriptorType
		{
			enum PageDescriptorType
			{
				INVALID = 0,
				RESERVED = 1,
				AVAILABLE = 2,
				ALLOCATED = 3,
			};
		}

		struct PageDescriptor
		{
			PageDescriptor *next_free;
			PageDescriptor *prev_free;
			PageDescriptorType::PageDescriptorType type;
		} __aligned(16);

		class MemoryManager;
		class ObjectAllocator;

		class PageAllocatorAlgorithm
		{
		public:
			virtual bool init(PageDescriptor *page_descriptors, uint64_t nr_page_descriptors) = 0;

			virtual void insert_page_range(PageDescriptor *start, uint64_t count) = 0;
			virtual void remove_page_range(PageDescriptor *start, uint64_t count) = 0;

			virtual PageDescriptor *allocate_pages(int order) = 0;
			virtual void free_pages(PageDescriptor *base, int order) = 0;

			virtual const char *name() const = 0;

			virtual void dump_state() const;
		};

		class PageAllocator : Allocator
		{
			friend class MemoryManager;
			friend class ObjectAllocator;

		public:
			PageAllocator(MemoryManager &mm);

			bool init() override;

			PageAllocatorAlgorithm *algorithm() const { return _allocator_algorithm; }
			void algorithm(PageAllocatorAlgorithm &alg) { _allocator_algorithm = &alg; }

			PageDescriptor *alloc_pages(int order);
			void free_pages(PageDescriptor *pgd, int order);

			inline const PageDescriptor *alloc_page() { return alloc_pages(0); }
			const PageDescriptor *alloc_zero_page();
			inline void free_page(PageDescriptor *pgd) { return free_pages(pgd, 0); }

			pfn_t pgd_to_pfn(const PageDescriptor *pgd) const
			{
				uintptr_t offset = (uintptr_t)pgd - (uintptr_t)_page_descriptors;
				offset /= sizeof(PageDescriptor);

				return (pfn_t)offset;
			}

			phys_addr_t pgd_to_pa(const PageDescriptor *pgd) const
			{
				return (phys_addr_t)pfn_to_pa(pgd_to_pfn(pgd));
			}

			virt_addr_t pgd_to_vpa(const PageDescriptor *pgd) const
			{
				return (virt_addr_t)pa_to_vpa(pgd_to_pa(pgd));
			}

			virt_addr_t pgd_to_kva(const PageDescriptor *pgd) const
			{
				return (virt_addr_t)pa_to_kva(pgd_to_pa(pgd));
			}

			PageDescriptor *pfn_to_pgd(pfn_t pfn) const
			{
				if (pfn > _nr_pages)
					return NULL;
				return &_page_descriptors[pfn];
			}

			PageDescriptor *vpa_to_pgd(virt_addr_t addr) const
			{
				phys_addr_t pa = vpa_to_pa(addr);
				pfn_t pfn = pa_to_pfn(pa);
				return pfn_to_pgd(pfn);
			}

		private:
			uint64_t _nr_pages;
			PageDescriptor *_page_descriptors;
			PageAllocatorAlgorithm *_allocator_algorithm;
			util::Mutex _mtx;

			bool setup_page_descriptors();
			bool self_test();
			uint64_t reserve_page_range(pfn_t start, uint64_t nr_pages);
		};

		extern infos::kernel::ComponentLog pgalloc_log;

#define RegisterPageAllocator(_class) \
	static _class __pgalloc_class;    \
	__section(".pgallocptr") infos::mm::PageAllocatorAlgorithm *__pgalloc_ptr_##_class = &__pgalloc_class
	}
}
