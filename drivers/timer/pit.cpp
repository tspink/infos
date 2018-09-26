/* SPDX-License-Identifier: MIT */

/*
 * drivers/timer/pit.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/timer/pit.h>
#include <arch/x86/pio.h>

using namespace infos::drivers;
using namespace infos::drivers::timer;
using namespace infos::arch::x86;

const DeviceClass PIT::PITDeviceClass(Timer::TimerDeviceClass, "pit");

PIT::PIT()
{
	
}

void PIT::start()
{
	uint8_t data = __inb(0x61);
	
	data &= 0xfe;
	__outb(0x61, data);
	data |= 1;
	__outb(0x61, data);
}

void PIT::stop()
{

}

void PIT::reset()
{

}

uint64_t PIT::count() const
{
	return 0;
}

bool PIT::expired() const
{
	uint8_t expired = __inb(0x61);
	return !(expired & 0x20);
}

void PIT::init_oneshot(uint64_t period)
{
	uint8_t data = __inb(0x61);
	data &= 0xfd;
	data |= 1;
	__outb(0x61, data);
	
	__outb(0x43, 0xb2);
	__outb(0x42, period);
	__outb(0x42, period >> 8);
}

void PIT::init_periodic(uint64_t period)
{

}
