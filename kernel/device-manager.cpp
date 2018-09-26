/* SPDX-License-Identifier: MIT */

/*
 * kernel/device-manager.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/device-manager.h>
#include <infos/kernel/irq.h>
#include <infos/kernel/log.h>
#include <infos/drivers/device.h>

using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::util;

ComponentLog dm_log(syslog, "devmgr");

DeviceManager::DeviceManager(Kernel& owner) : Subsystem(owner)
{
	
}

bool DeviceManager::register_device(drivers::Device& device)
{	
	uint64_t instance = device.device_class().acquire_instance();
	
	device.assign_name(String(device.device_class().name) + ToString(instance));
	
	dm_log.messagef(LogLevel::DEBUG, "registering device '%s'", device.name().c_str());
	_devices.add(device.name().get_hash(), &device);
		
	if (!device.init(*this)) {
		dm_log.messagef(LogLevel::ERROR, "device '%s' failed to initialise", device.name().c_str());
		return false;
	}
	
	return true;
}

bool DeviceManager::add_device_alias(const util::String& name, drivers::Device& device)
{
	// TODO: Check to make sure 'device' exists.
	dm_log.messagef(LogLevel::DEBUG, "registering device alias '%s' for '%s'", name.c_str(), device.name().c_str());
	_devices.add(name.get_hash(), &device);
	
	return true;
}
