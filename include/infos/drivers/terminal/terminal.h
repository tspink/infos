/* SPDX-License-Identifier: MIT */

/*
 * include/drivers/terminal/terminal.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */

#include <infos/drivers/device.h>
#include <infos/io/stream.h>
#include <infos/util/event.h>

namespace infos {
	namespace fs {
		class File;
	}

	namespace drivers {
		namespace console {
			class VirtualConsole;
			class PhysicalConsole;
		}
				
		namespace terminal {
			class Terminal : public drivers::Device, public io::Stream
			{
			public:
				static const DeviceClass TerminalDeviceClass;

				Terminal();
				virtual ~Terminal();
				
				const DeviceClass& device_class() const override { return TerminalDeviceClass; }
				
				void attach_output(console::VirtualConsole& console) { _attached_virt_console = &console; }
				void attach_input(console::PhysicalConsole& console) { _attached_phys_console = &console; }
				
				void append_to_read_buffer(uint8_t c);
				
				int read(void* buffer, size_t size) override;
				int write(const void* buffer, size_t size) override;
				
				bool supports_colour() const { return true; }
				
				fs::File* open_as_file() override;
				
			private:
				uint8_t _read_buffer[64];
				uint8_t _read_buffer_head, _read_buffer_tail;
				util::Event _read_buffer_event;
				
				console::VirtualConsole *_attached_virt_console;
				console::PhysicalConsole *_attached_phys_console;
			};
		}
	}
}
