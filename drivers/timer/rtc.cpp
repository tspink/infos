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
#include <infos/fs/file.h>

using namespace infos::drivers;
using namespace infos::drivers::timer;
using namespace infos::fs;

const DeviceClass RTC::RTCDeviceClass(Device::RootDeviceClass, "rtc");

struct userspace_tod_buffer
{
	unsigned short seconds, minutes, hours, day_of_month, month, year;
};

class RTCFile : public File
{
public:
	RTCFile(RTC &device) : device_(device) {}

	int pread(void *ubuffer, size_t size, off_t off) override
	{
		if (size < sizeof(userspace_tod_buffer))
		{
			return 0;
		}

		if (off != 0)
		{
			return 0;
		}

		RTCTimePoint tp;
		device_.read_timepoint(tp);

		userspace_tod_buffer *buffer = (userspace_tod_buffer *)ubuffer;
		buffer->day_of_month = tp.day_of_month;
		buffer->hours = tp.hours;
		buffer->minutes = tp.minutes;
		buffer->month = tp.month;
		buffer->seconds = tp.seconds;
		buffer->year = tp.year;

		return sizeof(*buffer);
	}

	int read(void *ubuffer, size_t size) override
	{
		if (size < sizeof(userspace_tod_buffer))
		{
			return 0;
		}

		RTCTimePoint tp;
		device_.read_timepoint(tp);

		userspace_tod_buffer *buffer = (userspace_tod_buffer *)ubuffer;
		buffer->day_of_month = tp.day_of_month;
		buffer->hours = tp.hours;
		buffer->minutes = tp.minutes;
		buffer->month = tp.month;
		buffer->seconds = tp.seconds;
		buffer->year = tp.year;

		return sizeof(*buffer);
	}

private:
	RTC &device_;
};

File *RTC::open_as_file()
{
	return new RTCFile(*this);
}
