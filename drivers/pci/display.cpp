/* SPDX-License-Identifier: MIT */

/*
 * drivers/pci/display.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/pci/display.h>

using namespace infos::drivers;
using namespace infos::drivers::pci;

Display::Display(PCIBus& bus, unsigned int slot, unsigned int func) : PCIDevice(bus, slot, func)
{

}
