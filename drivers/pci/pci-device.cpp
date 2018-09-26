/* SPDX-License-Identifier: MIT */

/*
 * drivers/pci/pci-device.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/pci/pci-device.h>
#include <infos/drivers/pci/pci-bus.h>
#include <infos/drivers/pci/bridge.h>
#include <infos/kernel/device-manager.h>
#include <arch/x86/pio.h>

#define PCI_CONFIG_ADDRESS	0xcf8
#define PCI_CONFIG_DATA		0xcfc

using namespace infos::drivers;
using namespace infos::drivers::pci;
using namespace infos::kernel;
using namespace infos::arch::x86;

const DeviceClass PCIDevice::PCIDeviceClass(Device::RootDeviceClass, "pci");

infos::kernel::ComponentLog infos::drivers::pci::pci_log(infos::kernel::syslog, "pci");

PCIDevice::PCIDevice(PCIBus& bus, unsigned int slot, unsigned int func) : _owner(bus), _slot(slot), _func(func)
{

}

PCIDevice::~PCIDevice()
{

}

PCIDeviceClass::PCIDeviceClass PCIDevice::pci_class() const
{
	return (PCIDeviceClass::PCIDeviceClass)((read_config(0x08) >> 24) & 0xff);
}

uint32_t PCIDevice::read_config(uint8_t reg) const
{
	return bus().read_config(_slot, _func, reg);
}

void PCIDevice::write_config(uint8_t reg, uint32_t value) const
{
	bus().write_config(_slot, _func, reg, value);
}
