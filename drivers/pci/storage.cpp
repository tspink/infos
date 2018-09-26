/* SPDX-License-Identifier: MIT */

/*
 * drivers/pci/storage.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/pci/storage.h>
#include <infos/drivers/ata/ata-controller.h>
#include <infos/kernel/device-manager.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/log.h>
#include <arch/arch.h>

using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::drivers::block;
using namespace infos::drivers::pci;
using namespace infos::drivers::ata;

const DeviceClass Storage::StorageDeviceClass(PCIDevice::PCIDeviceClass, "storage");

Storage::Storage(PCIBus& bus, unsigned int slot, unsigned int func) 
	: PCIDevice(bus, slot, func)
{

}

Storage::~Storage()
{

}

bool Storage::init(kernel::DeviceManager& dm)
{
	switch(subclass()) {
	case StorageSubclass::IDE_CONTROLLER:
		return init_ide_controller(dm);

	case StorageSubclass::SATA_CONTROLLER:
		return init_sata_controller(dm);
		
	default:
		pci_log.messagef(LogLevel::ERROR, "Unsupported PCI mass storage subclass %u", subclass());
		return false;
	}
}

StorageSubclass::StorageSubclass Storage::subclass() const
{
	uint32_t device_info = read_config(PCI_REG_INFO);
	return (StorageSubclass::StorageSubclass)(PCI_CONFIG_SUBCLASS(device_info));
}

bool Storage::init_ide_controller(DeviceManager& dm)
{
	IRQ *irq = NULL;
	
	uint32_t old_config = read_config(PCI_REG_IRQ);
	uint32_t new_config = (old_config & ~0xFF) | 0xFE;
	write_config(PCI_REG_IRQ, new_config);
	if ((read_config(PCI_REG_IRQ) & 0xFF) == 0xFE) {
		//irq = dm.owner().arch().request_irq();
		//if (!irq) return false;

		//pci_log.messagef(LogLevel::INFO, "Configuring IDE storage device with IRQ %u", irq->nr());
		new_config = (old_config & ~0xFF) | 0; //(uint8_t)irq->nr();
		write_config(PCI_REG_IRQ, new_config);
	} else {
		pci_log.messagef(LogLevel::ERROR, "Unsupported PCI IDE storage configuration");
		return false;
	}
	
	ATAControllerConfiguration cfg;
	cfg.BAR[0] = read_config(PCI_REG_BAR0);
	cfg.BAR[1] = read_config(PCI_REG_BAR1);
	cfg.BAR[2] = read_config(PCI_REG_BAR2);
	cfg.BAR[3] = read_config(PCI_REG_BAR3);
	cfg.BAR[4] = read_config(PCI_REG_BAR4);
	
	ATAController *dev = new ATAController(*irq, cfg);
	if (!dm.register_device(*dev)) {
		delete dev;
		return false;
	}
	
	return true;
}

bool Storage::init_sata_controller(kernel::DeviceManager& dm)
{
	pci_log.messagef(LogLevel::WARNING, "SATA controller not (yet!) supported");
	return true;
}
