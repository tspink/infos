/* SPDX-License-Identifier: MIT */

/*
 * include/arch/x86/cpu.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/kernel/cpu.h>

namespace infos
{
	namespace arch
	{
		namespace x86
		{
			class X86CPU : public infos::kernel::CPU
			{
			public:
				X86CPU();
			};
		}
	}
}
