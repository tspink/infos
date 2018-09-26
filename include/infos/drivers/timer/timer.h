/* SPDX-License-Identifier: MIT */

/*
 * include/drivers/timer/timer.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/drivers/device.h>

namespace infos
{
	namespace drivers
	{
		namespace timer
		{
			class Timer : public Device
			{
			public:
				static const DeviceClass TimerDeviceClass;
				
				const DeviceClass& device_class() const override { return TimerDeviceClass; }
				
				virtual void start() = 0;
				virtual void stop() = 0;
				virtual void reset() = 0;
				
				virtual void init_oneshot(uint64_t period) = 0;
				virtual void init_periodic(uint64_t period) = 0;
				
				virtual bool expired() const = 0;
				virtual uint64_t count() const = 0;
				virtual uint64_t frequency() const = 0;
			};
		}
	}
}
