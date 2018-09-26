/* SPDX-License-Identifier: MIT */

/*
 * drivers/pci/network.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/pci/network.h>

using namespace infos::drivers;
using namespace infos::drivers::pci;

Network::Network(PCIBus& bus, unsigned int slot, unsigned int func) : PCIDevice(bus, slot, func)
{

}
