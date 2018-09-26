/* SPDX-License-Identifier: MIT */

/*
 * drivers/console/console.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/console/console.h>

using namespace infos::drivers;
using namespace infos::drivers::console;

const DeviceClass Console::ConsoleDeviceClass(Device::RootDeviceClass, "console");

Console::Console()
{

}

Console::~Console()
{

}
