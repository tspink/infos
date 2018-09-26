/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/drivers/device.h>
#include <infos/kernel/log.h>

#define PCI_CONFIG_ADDRESS	0xcf8
#define PCI_CONFIG_DATA		0xcfc

#define PCI_CONFIG_VALUE(__v, __shift, __size) (((__v) >> (__shift)) & ((1 << __size) - 1))

#define PCI_REG_VENDOR	0x00
#define PCI_CONFIG_VENDOR(__v) PCI_CONFIG_VALUE(__v, 0, 16)
#define PCI_CONFIG_DEVICE(__v) PCI_CONFIG_VALUE(__v, 16, 16)

#define PCI_REG_INFO	0x08
#define PCI_CONFIG_CLASS(__v)		PCI_CONFIG_VALUE(__v, 24, 8)
#define PCI_CONFIG_SUBCLASS(__v)	PCI_CONFIG_VALUE(__v, 16, 8)

#define PCI_REG_CONFIG	0x0C
#define PCI_CONFIG_HDRTYPE(__v)	PCI_CONFIG_VALUE(__v, 16, 8)

#define PCI_REG_BUSINFO	0x18
#define PCI_CONFIG_PRIMARY_BUS(__v)		PCI_CONFIG_VALUE(__v, 0, 8)
#define PCI_CONFIG_SECONDARY_BUS(__v)	PCI_CONFIG_VALUE(__v, 8, 8)

#define PCI_REG_BAR0 0x10
#define PCI_REG_BAR1 0x14
#define PCI_REG_BAR2 0x18
#define PCI_REG_BAR3 0x1c
#define PCI_REG_BAR4 0x20
#define PCI_REG_BAR5 0x24

#define PCI_REG_IRQ 0x3c

namespace infos
{
	namespace drivers
	{
		namespace pci
		{
			class PCIBus;
			
			namespace PCIDeviceClass
			{
				enum PCIDeviceClass
				{
					NONE = 0,
					MASS_STORAGE = 1,
					NETWORK = 2,
					DISPLAY = 3,
					MULTIMEDIA = 4,
					MEMORY = 5,
					BRIDGE = 6,
					SIMPLE_COMM = 7,
					BASE_SYSTEM_PERIPHERALS = 8,
					INPUT = 9,
					DOCKING_STATION = 0xA,
					PROCESSOR = 0xB,
					SERIAL_BUS = 0xC,
					WIRELESS = 0xD,
					IIO = 0xE,
					SATELLITE = 0xF,
					CRYPTO = 0x10,
					SIGNAL_PROCESSING = 0x11,
					NA = 0xFF
				};
			}
			
			class PCIDevice : public Device
			{
			public:
				static const DeviceClass PCIDeviceClass;
				const DeviceClass& device_class() const override { return PCIDeviceClass; }
				
				PCIDevice(PCIBus& owner, unsigned int slot, unsigned int func);
				virtual ~PCIDevice();
				
				PCIBus& bus() const { return _owner; }
				
				unsigned int slot() const { return _slot; }
				unsigned int func() const { return _func; }
				
				PCIDeviceClass::PCIDeviceClass pci_class() const;
				
			protected:
				uint32_t read_config(uint8_t reg) const;
				void write_config(uint8_t reg, uint32_t value) const;
				
			private:
				PCIBus& _owner;
				unsigned int _slot, _func;
			};
			
			extern kernel::ComponentLog pci_log;
		}
	}
}
