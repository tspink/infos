/* SPDX-License-Identifier: MIT */

/*
 * kernel/om.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/om.h>

#include "infos/kernel/thread.h"

using namespace infos::kernel;

ObjectManager::ObjectManager(Kernel& owner) : Subsystem(owner), _next_handle(1)
{

}

ObjectHandle ObjectManager::register_object(Thread& owner, void* obj)
{
	util::UniqueLock<util::Mutex> l(_registry_lock);
	
	ObjectHandle handle(_next_handle++);
	
	ObjectDescriptor od;
	od.owner = &owner;
	od.object = obj;
	
	_objects.add(handle, od);
	
	return handle;
}

void* ObjectManager::get_object_secure(Thread& owner, ObjectHandle handle)
{
	ObjectDescriptor obj;
	if (!_objects.try_get_value(handle, obj)) {
		return NULL;
	}
	
	if (&obj.owner->owner() != &owner.owner()) {
		return NULL;
	}
	
	return obj.object;
}
