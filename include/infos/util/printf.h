/* SPDX-License-Identifier: MIT */

/*
 * include/util/printf.h
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
	namespace util
	{
		extern int snprintf(char *buffer, int size, const char *fmt, ...);
		extern int sprintf(char *buffer, const char *fmt, ...);
		extern int vsnprintf(char *buffer, int size, const char *fmt, va_list args);
	}
}
