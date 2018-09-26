/* SPDX-License-Identifier: MIT */

/*
 * drivers/pci/pci-bus.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/pci/pci-bus.h>
#include <infos/drivers/pci/pci-device.h>

#include <infos/drivers/pci/bridge.h>
#include <infos/drivers/pci/display.h>
#include <infos/drivers/pci/network.h>
#include <infos/drivers/pci/storage.h>

#include <infos/kernel/device-manager.h>

#include <arch/x86/pio.h>

using namespace infos::drivers;
using namespace infos::drivers::pci;
using namespace infos::kernel;
using namespace infos::arch::x86;

PCIBus::PCIBus(unsigned int bus_id) : _bus_id(bus_id)
{

}

bool PCIBus::probe(kernel::DeviceManager& dm)
{
	bool success = true;
	for (int slot = 0; slot < 32; slot++) {
		success &= probe_slot(dm, slot);
	}
	
	return success;
}

bool PCIBus::probe_slot(kernel::DeviceManager& dm, unsigned int slot)
{
	if (!probe_func(dm, slot, 0)) {
		pci_log.messagef(LogLevel::ERROR, "Probing slot %u func 0 failed.", slot);
		return false;
	}
	
	uint32_t device_cfg = read_config(slot, 0, PCI_REG_CONFIG);
	if (PCI_CONFIG_HDRTYPE(device_cfg) & 0x80) {
		for (int func = 1; func < 8; func++) {
			if (!probe_func(dm, slot, func)) {
				pci_log.messagef(LogLevel::ERROR, "Probing slot %u func %u failed.", slot, func);
				return false;
			}
		}
	}
	
	return true;
}

bool PCIBus::probe_func(kernel::DeviceManager& dm, unsigned int slot, unsigned int func)
{
	// A missing device is not an error.
	uint32_t device_vendor = read_config(slot, func, PCI_REG_VENDOR);	
	if (PCI_CONFIG_VENDOR(device_vendor) == 0xffff)
		return true;

	uint32_t device_info = read_config(slot, func, PCI_REG_INFO);

	PCIDeviceClass::PCIDeviceClass device_class = (PCIDeviceClass::PCIDeviceClass)PCI_CONFIG_CLASS(device_info);
	pci_log.messagef(LogLevel::DEBUG, "probing %d:%d:%d class=%u", _bus_id, slot, func, device_class);

	PCIDevice *new_device = NULL;
	switch (device_class) {
	case PCIDeviceClass::BRIDGE:
		new_device = new Bridge(*this, slot, func);
		break;
		
	case PCIDeviceClass::DISPLAY:
		new_device = new Display(*this, slot, func);
		break;

	case PCIDeviceClass::NETWORK:
		new_device = new Network(*this, slot, func);
		break;

	case PCIDeviceClass::MASS_STORAGE:
		new_device = new Storage(*this, slot, func);
		break;
		
	default:
		pci_log.messagef(LogLevel::WARNING, "Device class %u not (yet) supported", device_class);
		return true;
	}
	
	if (!new_device) {
		pci_log.messagef(LogLevel::ERROR, "Supported class %u failed to create device object", device_class);
		return false;
	}
	
	if (!dm.register_device(*new_device)) {
		pci_log.messagef(LogLevel::ERROR, "Supported class %u failed to register device object", device_class);
		
		delete new_device;
		return false;
	}
	
	return true;
}

uint32_t PCIBus::read_config(uint8_t slot, uint8_t func, uint8_t reg)
{
	uint32_t address = (0x80000000ULL |
			((uint32_t)_bus_id << 16) | 
			((uint32_t)slot << 11) | 
			((uint32_t)func << 8) |
			((uint32_t)reg & ~0x03ULL));

	__outl(PCI_CONFIG_ADDRESS, address);
	return __inl(PCI_CONFIG_DATA);
}

void PCIBus::write_config(uint8_t slot, uint8_t func, uint8_t reg, uint32_t value)
{
	uint32_t address = (0x80000000ULL |
			((uint32_t)_bus_id << 16) | 
			((uint32_t)slot << 11) | 
			((uint32_t)func << 8) |
			((uint32_t)reg & ~0x03ULL));

	__outl(PCI_CONFIG_ADDRESS, address);
	__outl(PCI_CONFIG_DATA, value);
}
