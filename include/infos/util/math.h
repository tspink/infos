/* SPDX-License-Identifier: MIT */

/*
 * include/infos/util/math.h
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
	namespace util
	{
		static inline uint32_t ilog2_floor(uint32_t v)
		{
			uint32_t r;
			asm("\tbsr %1, %0\n" : "=r"(r) : "r"(v));
			return r;
		}

		static inline uint32_t ilog2_ceil(uint32_t v)
		{
			uint32_t r;
			asm("\tbsr %1, %0\n" : "=r"(r) : "r"(v));

			return r + (v & (v - 1) ? 1 : 0);
		}
	}
}
