/* SPDX-License-Identifier: MIT */

/*
 * drivers/device.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/device.h>

using namespace infos::drivers;

const DeviceClass Device::RootDeviceClass;

Device::Device()
{

}

Device::~Device()
{

}

bool Device::init(kernel::DeviceManager& dm)
{
	return true;
}

infos::fs::File* Device::open_as_file()
{
	return NULL;
}
