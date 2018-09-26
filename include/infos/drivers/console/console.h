/* SPDX-License-Identifier: MIT */

/*
 * include/drivers/console/console.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/drivers/device.h>
#include <infos/drivers/input/keyboard.h>

namespace infos
{
	namespace drivers
	{
		namespace console
		{
			class Console : public drivers::Device, public input::KeyboardSink
			{
			public:
				static const DeviceClass ConsoleDeviceClass;

				Console();
				virtual ~Console();
				
				virtual bool supports_colour() const = 0;
				
				const DeviceClass& device_class() const override { return ConsoleDeviceClass; }
			};
		}
	}
}
