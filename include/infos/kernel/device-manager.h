/* SPDX-License-Identifier: MIT */

/*
 * include/kernel/device-manager.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/kernel/subsystem.h>
#include <infos/kernel/irq.h>
#include <infos/drivers/device.h>
#include <infos/util/list.h>
#include <infos/util/generator.h>
#include <infos/util/map.h>

namespace infos {
	namespace kernel {
		class DeviceManager : public Subsystem {
		public:
			DeviceManager(Kernel& owner);

			bool register_device(drivers::Device& device);
			bool add_device_alias(const util::String& name, drivers::Device& device);

			template<class T>
			bool try_get_device_by_class(const drivers::DeviceClass& device_class, T*& __out_device) const
			{
				for (auto dev : _devices) {
					if (dev.value->device_class().is(device_class)) {
						__out_device = (T*)dev.value;
						return true;
					}
				}
				
				return false;
			}
			
			template<class T>
			bool try_get_device_by_name(const util::String& name, T*& __out_device) const
			{
				drivers::Device *dev;
				if (!_devices.try_get_value(name.get_hash(), dev)) {
					return false;
				}
				
				__out_device = (T*)dev;
				return true;
			}
			
			const util::Map<util::String::hash_type, drivers::Device *>& devices() const { return _devices; }
			
		private:
			util::Map<util::String::hash_type, drivers::Device *> _devices;
		};
	}
}
