/* SPDX-License-Identifier: MIT */

/*
 * drivers/pci/bridge.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/pci/bridge.h>
#include <infos/drivers/pci/pci-bus.h>

using namespace infos::drivers;
using namespace infos::drivers::pci;
using namespace infos::kernel;

Bridge::Bridge(PCIBus& bus, unsigned int slot, unsigned int func) : PCIDevice(bus, slot, func)
{

}

bool Bridge::init(kernel::DeviceManager& dm)
{
	switch (subclass()) {
	case PCI_TO_PCI_BRIDGE:
	{
		unsigned int header_type = PCI_CONFIG_HDRTYPE(read_config(PCI_REG_CONFIG));
		
		if (header_type != 0x01 && header_type != 0x81) {
			pci_log.messagef(LogLevel::ERROR, "PCI-to-PCI Bridge has invalid header type %x", header_type);
			return false;
		}
		
		unsigned int secondary_bus_id = PCI_CONFIG_SECONDARY_BUS(read_config(PCI_REG_BUSINFO));
		pci_log.messagef(LogLevel::DEBUG, "PCI-to-PCI Bridge secondary=%d", subclass(), secondary_bus_id);
		
		_secondary = new PCIBus(secondary_bus_id);
		return _secondary->probe(dm);
	}
	
	default:
		pci_log.messagef(LogLevel::DEBUG, "PCI Bridge subclass=%u", subclass());
		break;
	}
	
	return true;
}

Bridge::Subclass Bridge::subclass() const
{
	return (Bridge::Subclass)PCI_CONFIG_SUBCLASS(read_config(PCI_REG_INFO));
}
