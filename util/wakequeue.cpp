/* SPDX-License-Identifier: MIT */

/*
 * util/wakequeue.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2021.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/util/event.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/thread.h>
#include <arch/arch.h>

using namespace infos::kernel;
using namespace infos::util;

void WakeQueue::sleep(Thread& thread)
{
    // TODO: some sort of lock
    _waiters.append(&thread);
    thread.sleep();
}

void WakeQueue::wake()
{
    // TODO: some sort of lock
    for (auto waiter : _waiters) {
        waiter->wake_up();
    }

    _waiters.clear();
}
