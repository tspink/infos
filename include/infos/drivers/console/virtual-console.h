/* SPDX-License-Identifier: MIT */

/*
 * include/drivers/console/virtual-console.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/drivers/console/console.h>

namespace infos
{
	namespace drivers
	{
		namespace terminal
		{
			class Terminal;
		}
		
		namespace console
		{
			class VirtualConsole : public Console
			{
			public:
				typedef void (*UpdateCallbackFn)(VirtualConsole& vc);
				
				static const DeviceClass VirtualConsoleDeviceClass;
				const DeviceClass& device_class() const override { return VirtualConsoleDeviceClass; }

				VirtualConsole();
				virtual ~VirtualConsole();
				
				void key_up(input::Keys::Keys key) override;
				void key_down(input::Keys::Keys key) override;
				
				int write(const void *buffer, size_t size);
				
				void set_buffer(uint16_t *buffer, UpdateCallbackFn ucb) { _buffer = buffer; _ucb = ucb; }
				uint16_t *get_buffer() const { return _buffer; }
				uint16_t get_buffer_position() const { return _current_pos; }
				
				void attach_terminal(terminal::Terminal *terminal);
				
				bool supports_colour() const override { return true; }
							
			private:
				enum KeyboardModifiers
				{
					None = 0,
					Ctrl = 1,
					Shift = 2,
					Alt = 4,
					CapsLock = 8
				};
				
				friend KeyboardModifiers operator|(const KeyboardModifiers& l, const KeyboardModifiers& r)
				{
					return (KeyboardModifiers)((uint32_t)l | (uint32_t)r);
				}
				
				friend KeyboardModifiers operator&(const KeyboardModifiers& l, const KeyboardModifiers& r)
				{
					return (KeyboardModifiers)((uint32_t)l & (uint32_t)r);
				}

				friend KeyboardModifiers operator~(const KeyboardModifiers& l)
				{
					return (KeyboardModifiers)(~(uint32_t)l);
				}
				
				KeyboardModifiers _current_mod_mask;
				
				constexpr static int _width = 80;
				constexpr static int _height = 25;
				
				uint16_t _current_pos;
				
				terminal::Terminal *_terminal;
				uint16_t *_buffer;
				UpdateCallbackFn _ucb;
				
				void scroll_one_line();
			};
		}
	}
}
