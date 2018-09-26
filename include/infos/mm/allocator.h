/* SPDX-License-Identifier: MIT */

/*
 * include/mm/allocator.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/define.h>

namespace infos
{
	namespace mm
	{
		class MemoryManager;
		
		class Allocator
		{
		public:
			Allocator(MemoryManager& mm) : _mm(mm) { }
			
			virtual bool init() = 0;
			
			MemoryManager& owner() const { return _mm; }
			
		private:
			MemoryManager& _mm;
		};
	}
}
