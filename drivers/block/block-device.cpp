/* SPDX-License-Identifier: MIT */

/*
 * drivers/block/block-device.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/block/block-device.h>

using namespace infos::drivers;
using namespace infos::drivers::block;

const DeviceClass BlockDevice::BlockDeviceClass(Device::RootDeviceClass, "block");
