/* SPDX-License-Identifier: MIT */

/*
 * drivers/timer/rtc.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2019.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/timer/rtc.h>

using namespace infos::drivers;
using namespace infos::drivers::timer;

const DeviceClass RTC::RTCDeviceClass(Device::RootDeviceClass, "rtc");
