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
    List<Core*> _cores = sys.device_manager().cores();
    for (Core *core : _cores) {
        // Initialise the BSP's scheduler
        if (core->get_state() == Core::core_state::BOOTSTRAP) {
            // todo: this is messy maybe fix this
            core->set_sched_ptr(&sys.scheduler());
            return core->get_sched_ptr()->init();
        }
    }

	return false;
}