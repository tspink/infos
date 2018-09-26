/* SPDX-License-Identifier: MIT */

/*
 * include/kernel/subsystem.h
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
	namespace kernel
	{
		class Kernel;
		
		class Subsystem
		{
		public:
			Subsystem(Kernel& owner) : _owner(owner) { }
			Kernel& owner() const { return _owner; }
			
		private:
			Kernel& _owner;
		};
	}
}
