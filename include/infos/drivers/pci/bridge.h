/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/drivers/pci/pci-device.h>

namespace infos
{
	namespace drivers
	{
		namespace pci
		{
			class Bridge : public PCIDevice
			{
			public:
				enum Subclass
				{
					HOST_BRIDGE = 0,
					ISA_BRIDGE = 1,
					EISA_BRIDGE = 2,
					MCA_BRIDGE = 3,
					PCI_TO_PCI_BRIDGE = 4,
					PCMCIA_BRIDGE = 5,
					NUBUS_BRIDGE = 6,
					CARDBUS_BRIDGE = 7,
					RACEWAY_BRIDGE = 8,
					PCI_TO_PCI_SEMITRANSPARENT = 9,
					INFINIBAND_TO_PCI_HOST_BRIDGE = 10,
					OTHER = 0x80
				};
				
				Bridge(PCIBus& bus, unsigned int slot, unsigned int func);
				
				bool init(kernel::DeviceManager& dm) override;
				
				Subclass subclass() const;
				
				PCIBus& secondary_bus() const { return *_secondary; }
				
			private:
				PCIBus *_secondary;
			};
		}
	}
}
