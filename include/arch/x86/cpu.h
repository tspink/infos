/* SPDX-License-Identifier: MIT */

/*
 * include/arch/x86/cpu.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/kernel/cpu.h>
#include <infos/drivers/timer/pit.h>
#include <infos/drivers/irq/lapic.h>
#include <infos/drivers/irq/ioapic.h>
#include <infos/drivers/irq/core.h>

namespace infos
{
	namespace arch
	{
		namespace x86
		{
			class X86CPU : public infos::kernel::CPU
			{
			public:
				X86CPU();
			};

            void start_core(infos::drivers::irq::Core* core, infos::drivers::irq::LAPIC* lapic, infos::drivers::timer::PIT* pit);

            extern kernel::ComponentLog cpu_log;
		}
	}
}
