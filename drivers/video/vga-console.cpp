/* SPDX-License-Identifier: MIT */

/*
 * drivers/video/vga-console.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/video/vga-console.h>
#include <infos/drivers/console/virtual-console.h>
#include <infos/kernel/device-manager.h>
#include <infos/util/string.h>
#include <arch/x86/pio.h>

using namespace infos::drivers;
using namespace infos::drivers::console;
using namespace infos::drivers::video;
using namespace infos::util;
using namespace infos::arch::x86;

const DeviceClass VGAConsoleDevice::VGAConsoleDeviceClass(PhysicalConsole::PhysicalConsoleDeviceClass, "vga");

VGAConsoleDevice::VGAConsoleDevice(phys_addr_t video_ram_addr) 
	: _video_ram_address(video_ram_addr),
		_backup_buffer(NULL),
		_old_vc(NULL)
{

}

VGAConsoleDevice::~VGAConsoleDevice()
{

}

void VGAConsoleDevice::virtual_console_changed()
{
	if (_old_vc) {
		_old_vc->set_buffer(_backup_buffer, NULL);
		memcpy(_old_vc->get_buffer(), (const void *)_video_ram_address, (80 * 25 * 2));
	}
	
	_backup_buffer = get_current_vc().get_buffer();
	get_current_vc().set_buffer((uint16_t *)_video_ram_address, VCUpdateCallback);
	
	if (_backup_buffer != NULL) {
		memcpy(get_current_vc().get_buffer(), _backup_buffer, (80 * 25 * 2));
	}
	
	_old_vc = &get_current_vc();
	VCUpdateCallback(get_current_vc());
}

void VGAConsoleDevice::VCUpdateCallback(console::VirtualConsole& vc)
{
	uint16_t position = vc.get_buffer_position();
	
	// Index register CP:LOW
	__outb(0x3d4, 0xf);
	__outb(0x3d5, position);

	// Index register CP:HI
	__outb(0x3d4, 0xe);
	__outb(0x3d5, (position >> 8));
}

void VGAConsoleDevice::update_cursor_position(unsigned int position)
{
	
}
