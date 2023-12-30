/* SPDX-License-Identifier: MIT */

/*
 * drivers/terminal/terminal.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/terminal/terminal.h>
#include <infos/drivers/console/virtual-console.h>
#include <infos/drivers/console/physical-console.h>
#include <infos/drivers/input/keyboard.h>
#include <infos/fs/file.h>
#include <infos/kernel/log.h>

using namespace infos::drivers;
using namespace infos::drivers::terminal;
using namespace infos::fs;
using namespace infos::kernel;

const DeviceClass Terminal::TerminalDeviceClass(Device::RootDeviceClass, "tty");

Terminal::Terminal()
	: _read_buffer_head(0),
		_read_buffer_tail(0)
{

}

ConsoleTerminal::ConsoleTerminal()
	: 	_attached_virt_console(NULL),
		_attached_phys_console(NULL)
{

}

SerialTerminal::SerialTerminal()
{

}

Terminal::~Terminal()
{

}

ConsoleTerminal::~ConsoleTerminal()
{

}

SerialTerminal::~SerialTerminal()
{

}

void Terminal::append_to_read_buffer(uint8_t c)
{
	_read_buffer[_read_buffer_tail++] = c;
	_read_buffer_tail %= ARRAY_SIZE(_read_buffer);
	_read_buffer_event.trigger();
}

int Terminal::read(void* raw_buffer, size_t size)
{
	if (size == 0) return 0;

	uint8_t *buffer = (uint8_t *)raw_buffer;
	size_t n = 0;
	while (n < size) {
		while (_read_buffer_head == _read_buffer_tail) {
			_read_buffer_event.wait();
		}

		uint8_t elem = _read_buffer[_read_buffer_head];

		_read_buffer_head++;
		_read_buffer_head %= ARRAY_SIZE(_read_buffer);

		buffer[n++] = elem;
	}

	return n;
}

int SerialTerminal::write(const void* buffer, size_t size)
{
	if (_attached_uart) {
		return _attached_uart->write(buffer, size);
	} else {
		return 0;
	}
}
void SerialTerminal::buffer_raw_character(uint8_t c)
{
	switch (c)
	{
		/* For a physical keyboard on the VGA console keyboard, the key 'Enter'
		 * generates '\n'. So translate the raw serial protocol to do the same. */
		case '\r': append_to_read_buffer('\n'); break;
		default:
			append_to_read_buffer(c); break;
	}
}

int ConsoleTerminal::write(const void* buffer, size_t size)
{
	if (_attached_virt_console) {
		return _attached_virt_console->write(buffer, size);
	} else {
		return 0;
	}
}

class TerminalFile : public File
{
public:
	TerminalFile(Terminal& tty) : _tty(tty) { }

	int read(void* buffer, size_t size) override
	{
		return _tty.read(buffer, size);
	}

	int write(const void* buffer, size_t size) override
	{
		return _tty.write(buffer, size);
	}

private:
	Terminal& _tty;
};

File* Terminal::open_as_file()
{
	return new TerminalFile(*this);
}
