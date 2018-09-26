/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/platform.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/init.h>
#include <arch/x86/dt.h>
#include <arch/x86/irq-entry-points.h>
#include <arch/x86/context.h>
#include <arch/arch.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/log.h>

using namespace infos::arch::x86;
using namespace infos::kernel;

