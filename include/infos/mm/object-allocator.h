/* SPDX-License-Identifier: MIT */

/*
 * include/mm/object-allocator.h
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
		namespace AllocFlags
		{
			enum AllocFlags
			{
				NONE = 0,
			};
		}

		class MemoryManager;
		
		class ObjectAllocator : Allocator
		{
			friend class MemoryManager;
			
		public:
			ObjectAllocator(MemoryManager& mm);
			
			bool init() override;
			
			void *alloc(size_t size, AllocFlags::AllocFlags flags = AllocFlags::NONE);
			void free(void *ptr);
			
		private:
			util::Mutex _mtx;
		};
		
		extern infos::kernel::ComponentLog objalloc_log;
	}
}
