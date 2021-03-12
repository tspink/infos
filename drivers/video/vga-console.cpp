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
#include <infos/drivers/video/video.h>
#include <infos/kernel/kernel.h>
#include <arch/x86/pio.h>

using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::drivers::console;
using namespace infos::drivers::video;
using namespace infos::util;
using namespace infos::arch::x86;

const DeviceClass VGAConsoleDevice::VGAConsoleDeviceClass(PhysicalConsole::PhysicalConsoleDeviceClass, "vga");

VGAConsoleDevice::VGAConsoleDevice() 
	: _video_ram_address(new uint16_t[80 * 25])
	, _backup_buffer(nullptr)
	, _old_vc(nullptr)
{

}

VGAConsoleDevice::~VGAConsoleDevice()
{
	delete _video_ram_address;
	get_current_vc().set_buffer(nullptr, nullptr);
	if (_old_vc) {
		_old_vc->set_buffer(nullptr, nullptr);
	}
	_video_ram_address = nullptr;
	_backup_buffer = nullptr;
}

void VGAConsoleDevice::virtual_console_changed()
{
	if (_old_vc) {
		_old_vc->set_buffer(_backup_buffer, nullptr);
		memcpy(_old_vc->get_buffer(), (const void *)_video_ram_address, (80 * 25 * 2));
	}
	
	_backup_buffer = get_current_vc().get_buffer();
	get_current_vc().set_buffer((uint16_t *)_video_ram_address, VCUpdateCallback);
	
	if (_backup_buffer != nullptr) {
		memcpy(get_current_vc().get_buffer(), _backup_buffer, (80 * 25 * 2));
	}
	
	_old_vc = &get_current_vc();
	VCUpdateCallback(get_current_vc());
}

void VGAConsoleDevice::VCUpdateCallback(console::VirtualConsole& vc)
{
	uint16_t position = vc.get_buffer_position();
	
	// Try to get video device
	VideoDevice *vd0;
	if(sys.device_manager().try_get_device_by_class(VideoDevice::VideoDeviceClass, vd0)) {
		vd0->draw_text(vc.get_buffer(), vc.width(), vc.height());
		return;
	}

	// Fallback to standard VGA if no video device found
	// Write back buffer to video ram
	memcpy((void *)pa_to_vpa(0xb8000), vc.get_buffer(), 80 * 25 * 2);

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
