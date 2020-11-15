/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/kernel/log.h>
#include <arch/x86/core.h>

namespace infos
{
	namespace arch
	{
		namespace x86
		{
			namespace acpi
			{
				bool acpi_init();
				uint32_t acpi_get_ioapic_base();
				Core* acpi_get_cores();
				uint8_t acpi_get_num_cores();

				extern kernel::ComponentLog acpi_log;
			}
		}
	}
}
