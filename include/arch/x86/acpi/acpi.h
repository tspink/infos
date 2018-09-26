/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/kernel/log.h>

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
				
				extern kernel::ComponentLog acpi_log;
			}
		}
	}
}
