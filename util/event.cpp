/* SPDX-License-Identifier: MIT */

/*
 * util/event.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/util/event.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/thread.h>
#include <arch/arch.h>

using namespace infos::kernel;
using namespace infos::util;

void Event::trigger()
{
	_wakequeue.wake();
}

void Event::wait()
{
	_wakequeue.sleep(Thread::current());
}
