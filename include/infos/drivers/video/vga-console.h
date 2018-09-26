/* SPDX-License-Identifier: MIT */

/*
 * include/drivers/video/vga-console.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/drivers/console/physical-console.h>

namespace infos
{
	namespace drivers
	{
		namespace console
		{
			class VirtualConsole;
		}
		
		namespace video
		{
			class VGAConsoleDevice : public console::PhysicalConsole
			{
			public:
				static const DeviceClass VGAConsoleDeviceClass;
				const DeviceClass& device_class() const override { return VGAConsoleDeviceClass; }
				
				VGAConsoleDevice(phys_addr_t video_ram_address);
				virtual ~VGAConsoleDevice();
				
				bool supports_colour() const override { return true; }
			
			protected:
				void virtual_console_changed() override;

			private:
				phys_addr_t _video_ram_address;
				
				uint16_t *_backup_buffer;
				console::VirtualConsole *_old_vc;
				
				static void VCUpdateCallback(console::VirtualConsole& vc);
				void update_cursor_position(unsigned int position);
			};
		}
	}
}
