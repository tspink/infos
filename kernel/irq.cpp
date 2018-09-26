/* SPDX-License-Identifier: MIT */

/*
 * kernel/irq.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/irq.h>
#include <infos/kernel/kernel.h>
#include <arch/arch.h>

using namespace infos::kernel;

bool IRQ::invoke() const
{
	if (_handler) {
		_handler(this, _priv);
		return true;
	} else {
		return false;
	}
}
