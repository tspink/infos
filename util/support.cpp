/* SPDX-License-Identifier: MIT */

/*
 * util/support.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/kernel.h>
#include <infos/kernel/log.h>
#include <infos/mm/mm.h>
#include <infos/mm/object-allocator.h>
#include <infos/util/string.h>

using namespace infos::kernel;
using namespace infos::mm;
using namespace infos::util;

void *operator new(size_t size)
{
	return sys.mm().objalloc().alloc(size, AllocFlags::NONE);
}

void *operator new[](size_t size)
{
	return sys.mm().objalloc().alloc(size, AllocFlags::NONE);
}

void operator delete(void *p)
{
	sys.mm().objalloc().free(p);
}

void operator delete(void *p, size_t sz)
{
	sys.mm().objalloc().free(p);
}

extern "C" {

	void __cxa_pure_virtual()
	{
		arch_abort();
	}

	void *__dso_handle;

	int __cxa_atexit(void (*destructor) (void *), void *arg, void *dso)
	{
		return 0;
	}

	void __cxa_finalize(void *f)
	{
	}
	
	void __stack_chk_fail()
	{
		arch_abort();
	}
}
