/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/kernel/kernel.h>
#include <infos/mm/mm.h>
#include <infos/mm/object-allocator.h>

namespace infos
{
	namespace util
	{
		class Allocator
		{
		public:
			virtual void *alloc(size_t size) = 0;
			virtual void free(void *ptr) = 0;
		};
		
		class DefaultAllocator : public Allocator
		{
		public:
			void* alloc(size_t size) override
			{
				return infos::kernel::sys.mm().objalloc().alloc(size);
			}
			
			void free(void* ptr) override
			{
				return infos::kernel::sys.mm().objalloc().free(ptr);
			}
		};
	}
}
