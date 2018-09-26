/* SPDX-License-Identifier: MIT */

/*
 * drivers/timer/timer.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/timer/timer.h>

using namespace infos::drivers;
using namespace infos::drivers::timer;

const DeviceClass Timer::TimerDeviceClass(Device::RootDeviceClass, "timer");