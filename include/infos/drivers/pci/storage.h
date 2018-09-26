/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/drivers/pci/pci-device.h>
#include <infos/drivers/block/block-device.h>

namespace infos
{
	namespace kernel
	{
		class IRQ;
	}
	
	namespace drivers
	{
		namespace pci
		{
			namespace StorageSubclass
			{
				enum StorageSubclass
				{
					SCSI_BUS_CONTROLLER = 0,
					IDE_CONTROLLER = 1,
					FLOPPY_DISK_CONTROLLER = 2,
					IPI_BUS_CONTROLLER = 3,
					RAID_CONTROLLER = 4,
					ATA_CONTROLLER = 5,
					SATA_CONTROLLER = 6,
					SAS_CONTROLLER = 7,
					OTHER = 0x80,
				};
			}
			
			class Storage : public PCIDevice
			{
			public:
				static const DeviceClass StorageDeviceClass;
				const DeviceClass& device_class() const override { return StorageDeviceClass; }

				Storage(PCIBus& bus, unsigned int slot, unsigned int func);
				virtual ~Storage();
				
				bool init(kernel::DeviceManager& dm) override;
				
				StorageSubclass::StorageSubclass subclass() const;
				
			private:
				bool init_ide_controller(kernel::DeviceManager& dm);
				bool init_sata_controller(kernel::DeviceManager& dm);
			};
		}
	}
}
