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
			X86Context *native_context;		// 0
			uintptr_t kernel_stack;			// 8
			uintptr_t xsave_area;			// 16
		} __packed;
	}
}
