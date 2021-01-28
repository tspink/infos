/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/sched.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/init.h>
#include <arch/arch.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/log.h>
#include <infos/drivers/irq/core.h>
#include <infos/util/list.h>

using namespace infos::arch::x86;
using namespace infos::kernel;
using namespace infos::drivers::irq;
using namespace infos::util;

bool infos::arch::x86::sched_init()
{
    // Initialise the BSP's scheduler
    return Core::get_current_core()->sched_init();
}