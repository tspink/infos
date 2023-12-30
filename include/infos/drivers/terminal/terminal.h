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
#include <arch/x86/uart.h> /* HACK */

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
				
				void append_to_read_buffer(uint8_t c);
				
				int read(void* buffer, size_t size) override;
				virtual bool supports_colour() const = 0;
				
				fs::File* open_as_file() override;
				
			private:
				uint8_t _read_buffer[64];
				uint8_t _read_buffer_head, _read_buffer_tail;
				util::Event _read_buffer_event;
			};
			class ConsoleTerminal : public Terminal
			{
			public:
				ConsoleTerminal();
				virtual ~ConsoleTerminal();
				
				void attach_output(console::VirtualConsole& console) { _attached_virt_console = &console; }
				void attach_input(console::PhysicalConsole& console) { _attached_phys_console = &console; }

				int write(const void* buffer, size_t size) override;

				bool supports_colour() const { return true; }

			private:
				console::VirtualConsole *_attached_virt_console;
				console::PhysicalConsole *_attached_phys_console;
			};
			class SerialTerminal : public Terminal
			{
			public:
				SerialTerminal();
				virtual ~SerialTerminal();

				/* FIXME: UART should not be in arch::x86 */
				void attach_uart(arch::x86::UART& uart) { _attached_uart = &uart; }

				int write(const void* buffer, size_t size) override;
				/* Serial line protocols might require us to lightly cook the raw chars. */
				void buffer_raw_character(uint8_t c);

				bool supports_colour() const { return true; }

			private:
				arch::x86::UART *_attached_uart;
			};
		}
	}
}
