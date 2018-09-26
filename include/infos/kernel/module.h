/* SPDX-License-Identifier: MIT */

/*
 * include/kernel/module.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/kernel/subsystem.h>

namespace infos
{
	namespace kernel
	{
		class ModuleManager : public Subsystem
		{
		public:
			ModuleManager(Kernel& owner);
			
			bool LoadModule(void *module_addr, size_t length);
		};
	}
}
