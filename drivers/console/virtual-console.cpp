/* SPDX-License-Identifier: MIT */

/*
 * drivers/console/virtual-console.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/console/virtual-console.h>
#include <infos/drivers/terminal/terminal.h>
#include <infos/kernel/kernel.h>
#include <infos/util/string.h>

using namespace infos::drivers;
using namespace infos::drivers::console;
using namespace infos::drivers::input;
using namespace infos::util;
using namespace infos::fs;

const DeviceClass VirtualConsole::VirtualConsoleDeviceClass(Console::ConsoleDeviceClass, "vc");

VirtualConsole::VirtualConsole() : _current_mod_mask(None), _current_pos(0), _buffer(NULL), _ucb(NULL)
{
	_buffer = new uint16_t[_width * _height];

	for (int i = 0; i < _width * _height; i++)
	{
		_buffer[i] = 0x0700;
	}
}

VirtualConsole::~VirtualConsole()
{
	delete _buffer;
}

void VirtualConsole::attach_terminal(terminal::Terminal *terminal)
{
	_terminal = terminal;
	_terminal->attach_output(*this);
}

void VirtualConsole::key_up(Keys::Keys key)
{
	switch (key)
	{
	case Keys::KEY_LSHIFT:
	case Keys::KEY_RSHIFT:
		_current_mod_mask = _current_mod_mask & ~Shift;
		return;

	default:
		break;
	}
}

void VirtualConsole::key_down(Keys::Keys key)
{
#define KEY_MAPPING(_kc, _upper, _lower) \
	case Keys::_kc:                      \
		if (_current_mod_mask & Shift)   \
			ch = _upper;                 \
		else                             \
			ch = _lower;                 \
		break
	char ch = 0;
	switch (key)
	{
		KEY_MAPPING(KEY_A, 'A', 'a');
		KEY_MAPPING(KEY_B, 'B', 'b');
		KEY_MAPPING(KEY_C, 'C', 'c');
		KEY_MAPPING(KEY_D, 'D', 'd');
		KEY_MAPPING(KEY_E, 'E', 'e');
		KEY_MAPPING(KEY_F, 'F', 'f');
		KEY_MAPPING(KEY_G, 'G', 'g');
		KEY_MAPPING(KEY_H, 'H', 'h');
		KEY_MAPPING(KEY_I, 'I', 'i');
		KEY_MAPPING(KEY_J, 'J', 'j');
		KEY_MAPPING(KEY_K, 'K', 'k');
		KEY_MAPPING(KEY_L, 'L', 'l');
		KEY_MAPPING(KEY_M, 'M', 'm');
		KEY_MAPPING(KEY_N, 'N', 'n');
		KEY_MAPPING(KEY_O, 'O', 'o');
		KEY_MAPPING(KEY_P, 'P', 'p');
		KEY_MAPPING(KEY_Q, 'Q', 'q');
		KEY_MAPPING(KEY_R, 'R', 'r');
		KEY_MAPPING(KEY_S, 'S', 's');
		KEY_MAPPING(KEY_T, 'T', 't');
		KEY_MAPPING(KEY_U, 'U', 'u');
		KEY_MAPPING(KEY_V, 'V', 'v');
		KEY_MAPPING(KEY_W, 'W', 'w');
		KEY_MAPPING(KEY_X, 'X', 'x');
		KEY_MAPPING(KEY_Y, 'Y', 'y');
		KEY_MAPPING(KEY_Z, 'Z', 'z');

		KEY_MAPPING(KEY_0, ')', '0');
		KEY_MAPPING(KEY_1, '!', '1');
		KEY_MAPPING(KEY_2, '"', '2');
		KEY_MAPPING(KEY_3, '#', '3');
		KEY_MAPPING(KEY_4, '$', '4');
		KEY_MAPPING(KEY_5, '%', '5');
		KEY_MAPPING(KEY_6, '^', '6');
		KEY_MAPPING(KEY_7, '&', '7');
		KEY_MAPPING(KEY_8, '*', '8');
		KEY_MAPPING(KEY_9, '(', '9');

		KEY_MAPPING(KEY_HYPHEN, '_', '-');
		KEY_MAPPING(KEY_EQUALS, '+', '=');
		KEY_MAPPING(KEY_BACKSLASH, '|', '\\');
		KEY_MAPPING(KEY_FORSLASH, '?', '/');

		KEY_MAPPING(KEY_DOT, '>', '.');

	case Keys::KEY_SPACE:
		ch = ' ';
		break;

	case Keys::KEY_RETURN:
		ch = '\n';
		break;

	case Keys::KEY_BACKSPACE:
		ch = '\b';
		break;

	case Keys::KEY_LSHIFT:
	case Keys::KEY_RSHIFT:
		_current_mod_mask = _current_mod_mask | Shift;
		return;

	default:
		return;
	}

	_terminal->append_to_read_buffer(ch);
}

int VirtualConsole::write(const void *buffer, size_t size)
{
	if (!_buffer)
		return 0;

	int attr = 0x0700;
	int state = 0;
	for (unsigned int i = 0; i < size; i++)
	{
		char c = ((uint8_t *)buffer)[i];

		if (state == 1)
		{
			attr = (int)c << 8;
			state = 0;
		}
		else if (state == 0)
		{
			if (c == '\n')
			{
				_current_pos += _width - (_current_pos % _width);
			}
			else if (c == '\r')
			{
				_current_pos -= (_current_pos % _width);
			}
			else if (c == '\33')
			{
				state = 1;
			}
			else if (c == '\b')
			{
				_current_pos--;
				_buffer[_current_pos] = attr | ' ';
			}
			else
			{
				_buffer[_current_pos] = attr | c;
				_current_pos++;
			}

			if (_current_pos >= _width * _height)
			{
				scroll_one_line();
				_current_pos = _width * (_height - 1);
			}
		}
	}

	if (_ucb)
	{
		_ucb(*this);
	}

	return size;
}

void VirtualConsole::scroll_one_line()
{
	memcpy(_buffer, &_buffer[_width], _width * (_height - 1) * 2);

	for (int x = 0; x < _width; x++)
	{
		_buffer[x + (_width * (_height - 1))] = 0x0700;
	}
}

File *VirtualConsole::open_as_file()
{
	return new VirtualConsoleFile(*this);
}

int VirtualConsoleFile::pwrite(const void *buffer, size_t size, off_t offset)
{
	if (size != 2)
	{
		return 0;
	}

	vc_._buffer[offset] = *(uint16_t *)buffer;
	return 2;
}
