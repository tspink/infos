/* SPDX-License-Identifier: MIT */

/*
 * include/mm/mm.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/kernel/subsystem.h>
#include <infos/mm/page-allocator.h>
#include <infos/mm/object-allocator.h>

namespace infos
{
	namespace kernel
	{
		class ComponentLog;
	}
	
	namespace mm
	{
		namespace MemoryType
		{
			enum MemoryType
			{
				NORMAL,
				UNUSABLE
			};
		}
				
		struct PhysicalMemoryBlock
		{
			phys_addr_t base_address;
			pfn_t base_pfn;
			unsigned int nr_pages;
			MemoryType::MemoryType type;
		};
		
		class PageAllocator;
		class ObjectAllocator;
		
		class MemoryManager : public kernel::Subsystem
		{
			friend class PageAllocator;
			friend class ObjectAllocator;
			
		public:
			MemoryManager(kernel::Kernel& owner);
			
			void add_physical_memory(phys_addr_t addr, unsigned int nr_pages, MemoryType::MemoryType type);
			bool initialise_allocators();
			
			PageAllocator& pgalloc() { return _page_alloc; }
			ObjectAllocator& objalloc() { return _obj_alloc; }
			
		private:
			static constexpr unsigned int _page_size = 0x1000;
			static constexpr unsigned int _page_bits = 12;
			pfn_t _last_pfn;
			
			unsigned int _nr_phys_mem_blocks;
			PhysicalMemoryBlock _phys_mem_blocks[16];
			
			const PhysicalMemoryBlock *lookup_phys_block(phys_addr_t addr);
			
			PageAllocator _page_alloc;
			ObjectAllocator _obj_alloc;
			
			bool test_page_allocator_order(int order);
			bool test_page_allocator();
			
			PageAllocatorAlgorithm *acquire_page_allocator_algorithm();
		};
		
		extern kernel::ComponentLog mm_log;
	}
}
