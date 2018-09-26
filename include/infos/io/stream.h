/* SPDX-License-Identifier: MIT */

/*
 * include/io/stream.h
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
	namespace io
	{
		class Stream
		{
		public:
			virtual int write(const void *buffer, size_t size) = 0;
			virtual int read(void *buffer, size_t size) = 0;
		};
	}
}
