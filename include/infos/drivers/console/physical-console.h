/* SPDX-License-Identifier: MIT */

/*
 * include/drivers/console/physical-console.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/drivers/console/console.h>
#include <infos/util/list.h>

namespace infos
{
	namespace drivers
	{
		namespace console
		{
			class VirtualConsole;
			
			class PhysicalConsole : public Console
			{
			public:
				PhysicalConsole();
				
				static const DeviceClass PhysicalConsoleDeviceClass;
				const DeviceClass& device_class() const override { return PhysicalConsoleDeviceClass; }
				
				void key_up(input::Keys::Keys key) override;
				void key_down(input::Keys::Keys key) override;
				
				void add_virtual_console(VirtualConsole& vc);
				void set_current_vc(VirtualConsole& vc) { _current_vc = &vc; virtual_console_changed(); }
				VirtualConsole& get_current_vc() const { return *_current_vc; }
				
			protected:
				virtual void virtual_console_changed() = 0;
				
			private:
				input::Keyboard *_kbd;
				util::List<VirtualConsole *> _vcs;
				VirtualConsole *_current_vc;
				bool _alt_pressed;
			};
		}
	}
}
