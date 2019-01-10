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
			struct RTCTimePoint
			{
				unsigned short seconds, minutes, hours, day_of_month, month, year;
			};
						
			class RTC : public Device
			{
			public:
				static const DeviceClass RTCDeviceClass;
				
				const DeviceClass& device_class() const override { return RTCDeviceClass; }
				
				virtual void read_timepoint(RTCTimePoint& tp) = 0;
				
				fs::File* open_as_file() override;
			};
		}
	}
}
