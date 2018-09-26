/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/define.h>
#include <infos/util/list.h>

namespace infos
{
	namespace mm
	{
		class PageDescriptor;
		
		namespace MappingFlags
		{
			enum MappingFlags
			{
				None		= 0,
				Present		= 1,
				User		= 2,
				Writable	= 4
			};
			
			static inline MappingFlags operator|(const MappingFlags& l, const MappingFlags& r)
			{
				return (MappingFlags)((uint64_t)l | (uint64_t)r);
			}
		}
		
		class VMA
		{
		public:
			VMA();
			virtual ~VMA();
			
			phys_addr_t pgt_base() const { return _pgt_phys_base; }
			
			PageDescriptor *allocate_phys(int order);
			bool allocate_virt(virt_addr_t va, int nr_pages);
			bool allocate_virt_any(int nr_pages);
			
			void insert_mapping(virt_addr_t va, phys_addr_t pa, MappingFlags::MappingFlags flags);
			bool get_mapping(virt_addr_t va, phys_addr_t& pa);
			bool is_mapped(virt_addr_t va);
			
			void install_default_kernel_mapping();
			
			bool copy_to(virt_addr_t dest_va, const void *src, size_t size);
			
			void dump();
					
		private:
			struct PageAllocation
			{
				PageDescriptor *descriptor_base;
				int allocation_order;
			};
			
			util::List<PageAllocation> _page_allocations;
			
			phys_addr_t _pgt_phys_base;
			virt_addr_t _pgt_virt_base;
			
			void dump_pdp(int pml4, virt_addr_t pdp_va);
			void dump_pd(int pml4, int pdp, virt_addr_t pd_va);
			void dump_pt(int pml4, int pdp, int pd, virt_addr_t pt_va);
		};
	}
}
