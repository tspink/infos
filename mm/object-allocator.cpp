/* SPDX-License-Identifier: MIT */

/*
 * mm/object-allocator.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/mm/object-allocator.h>
#include <infos/mm/mm.h>
#include <infos/util/lock.h>
#include <infos/util/cmdline.h>
#include <infos/util/string.h>

using namespace infos::mm;
using namespace infos::kernel;
using namespace infos::util;

ComponentLog infos::mm::objalloc_log(syslog, "objalloc");

RegisterCmdLineArgument(ObjAllocDebug, "objalloc.debug") {
	if (strncmp(value, "1", 1) == 0) {
		objalloc_log.enable();
	} else {
		objalloc_log.disable();
	}
}

ObjectAllocator::ObjectAllocator(MemoryManager& mm) : Allocator(mm)
{
}

bool ObjectAllocator::init()
{
	return true;
}

extern "C" void *dlmalloc(size_t size);
extern "C" void dlfree(void *ptr);

void *ObjectAllocator::alloc(size_t size, AllocFlags::AllocFlags flags)
{
	UniqueLock<Mutex> l(_mtx);
	
	void *ptr = dlmalloc(size);
	
	objalloc_log.messagef(LogLevel::DEBUG, "alloc: %lu (%u) = %p", size, flags, ptr);
	return ptr;
}

void ObjectAllocator::free(void* ptr)
{
	UniqueLock<Mutex> l(_mtx);
	
	objalloc_log.messagef(LogLevel::DEBUG, "free: %p", ptr);
	dlfree(ptr);
}
