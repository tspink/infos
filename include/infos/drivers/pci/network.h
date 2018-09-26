/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/drivers/pci/pci-device.h>

namespace infos
{
	namespace drivers
	{
		namespace pci
		{
			class Network : public PCIDevice
			{
			public:
				Network(PCIBus& bus, unsigned int slot, unsigned int func);
			};
		}
	}
}
