/* SPDX-License-Identifier: MIT */

/*
 * include/infos/kernel/cpu.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/kernel/kernel.h>
#include <arch/arch.h>

namespace infos
{
	namespace kernel
	{
		class CPU
		{
		public:
			static CPU& current() {
				return sys.arch().get_current_cpu();
			}
		};
	}
}
