/* SPDX-License-Identifier: MIT */

/*
 * kernel/module.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/module.h>

using namespace infos::kernel;

ModuleManager::ModuleManager(Kernel& owner) : Subsystem(owner)
{

}

bool ModuleManager::LoadModule(void* module_addr, size_t length)
{
	return false;
}
