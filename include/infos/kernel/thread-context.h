/* SPDX-License-Identifier: MIT */

/*
 * include/kernel/thread-context.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

// HACK HACK HACK
#include <arch/x86/context.h>

namespace infos
{
	namespace kernel
	{
		struct ThreadContext
		{
			X86Context *native_context;
			uintptr_t kernel_stack;
		} __packed;
	}
}
