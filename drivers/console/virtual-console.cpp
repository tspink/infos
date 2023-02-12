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

VirtualConsole::VirtualConsole() : _current_mod_mask(None), _current_pos(0),
	_escape_nchars(0), _attr_byte(0x07), _buffer(NULL), _ucb(NULL)
{
	_buffer = new uint16_t[_width * _height];
	for (int i = 0; i < _width * _height; i++)
	{
		_buffer[i] = 0x0700;
	}
	memset(_escape_buffer, 0, sizeof _escape_buffer);
}

VirtualConsole::~VirtualConsole()
{
	delete _buffer;
}

void VirtualConsole::attach_terminal(terminal::ConsoleTerminal *terminal)
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

uint8_t VirtualConsole::apply_ansi_specifier(int code)
{
	/* Update our attribute byte based on the following supported ANSI specifiers.
		 0      reset attributes
		 5      blink (slow)
		 7      inverse video
		25      blink off
		27      inverse off
		30--37  set foreground color (3-bit)
		39      set default foreground color
		40--47  set background color
		49      set default background color
		90--97  set bright foreground color

	 ANSI low-intensity colours are in bit order 'BGR'
		0	Black        000 (RGB)
		1	Red          100
		2	Green        010
		3	Yellow       110
		4	Blue         001
		5	Magenta      101
		6	Cyan         011
		7	Grey/white   111

	 BIOS low-intensity colours are more sensibly 'RGB'.
		0	Black        000
		1	Blue         001  <-- different
		2	Green        010
		3	Cyan         011  <-- different
		4	Red          100  <-- different
		5	Magenta      101
		6	Brown        110  <-- different
		7	Grey/white   111

	 To convert (symmetric), we swap bit 0 with bit 2.
	 */
	// FIXME: enumify/prettify this
#define BLINK 0x80
#define INVERT(x)  ( (((x)&0x7) << 4) | (((x)&(0x7<<4))>>4) | ((x) & BLINK) )
#define CONVERT_3BIT(x)  ( (((x) & 1) << 2) | ((x) & 2) | (((x) & 4) >> 2) )
	switch (code)
	{
		case 0:   /* reset */      _attr_byte = 0x07; break;
		case 5:   /* blink */      _attr_byte |= BLINK; break;
		case 7:   /* reverse */    _attr_byte = INVERT(_attr_byte); break;
		case 25:  /* no blink */   _attr_byte &= ~BLINK; break;
		case 27:  /* no reverse */ _attr_byte = INVERT(_attr_byte); break;
		// foreground -- low 4 bits
		case 30 ... 37:
			_attr_byte &= ~0xf; _attr_byte |= CONVERT_3BIT(code-30);
			break;
		case 39: /* default foreground colour is 0x7 */
			_attr_byte &= ~0xf; _attr_byte |= 0x7; break;
		// background: bits 4..6
		case 40 ... 47:
			_attr_byte &= ~(0x7<<4); _attr_byte |= (CONVERT_3BIT(code-40)<<4);
			break;
		case 49: /* default background colour is 0, i.e. just clear the bits */
			_attr_byte &= ~(0x7<<4); break;
		// foreground again, but with intensity
		case 90 ... 97:
			_attr_byte &= ~0xf; _attr_byte |= (0x08u | (CONVERT_3BIT(code-90)));
			break;
		default:
			break;
	}
	return _attr_byte;
}
void VirtualConsole::parse_escape_buffer()
{
	int decimal = -1;
	assert(_escape_nchars > 1);
	assert(_escape_buffer[0] == '\033');
	assert(_escape_buffer[1] == '[');
	for (int pos = 2; pos < _escape_nchars; ++pos)
	{
		char e = _escape_buffer[pos];
		switch (e)
		{
			case ';':
			case 'm':
				if (decimal != -1) apply_ansi_specifier(decimal);
				decimal = -1;
				break;
			case '0' ... '9':
				decimal = 10 * ((decimal == -1) ? 0 : decimal) + (e - '0');
				break;
			default: assert(false);
		}
	}
}
int VirtualConsole::write(const void *buffer, size_t size)
{
	if (!_buffer)
		return 0;

	for (unsigned int i = 0; i < size; i++)
	{
		char c = ((uint8_t *)buffer)[i];

		/* Invariant: if in the escaped state, it means we've seen a \033 followed by
		 * zero or more other characters that do NOT (yet) make a parseable supported
		 * ANSI escape code, and that there is room for at least one more char
		 * in our escape buffer. */
		if (_escape_nchars)
		{
			assert(_escape_nchars < sizeof _escape_buffer);
			/* For now we just support that subset of the Select Graphic Rendition
			 * escape codes that correspond to VGA features, i.e. colour and 'blink'
			 * and reverse video. These have the form
			 * \033[Xm
			 *
			 * where X is a semicolon-separated list drawn from the specifiers (see
			 * comment in apply_ansi_specifier, above). Each specifier is given as
			 * a decimal number, i.e. a sequence drawn from 0..9.
			 *
			 * Other specifiers may be included but will be ignored. So e.g.
			 *
			 * ^[ [ 3 4 ; 1 m
			 *
			 * will set the foreground colour to blue. The '1' also requests bold,
			 * but we ignore it.
			 *
			 * To parse, just look for an initial '[' and if seen, gobble digits or semicolons
			 * until we see 'm'.
			 */
			bool do_parse = false;
			if (_escape_nchars == 1 && c == '[') goto append;
			if (_escape_nchars >= 2 && c == 'm') { do_parse = true; goto append; }
			if (_escape_nchars >= 2 && c >= '0' && c <= '9') goto append;
			if (_escape_nchars >= 2 && c == ';') goto append;
			// if not ';' or 0-9 or 'm', we can reset
			goto reset;
		append:
			_escape_buffer[_escape_nchars++] = c;
			if (do_parse) parse_escape_buffer();
			else if (_escape_nchars != sizeof _escape_buffer) continue;
			// else we're out of space; fall through from parse to reset
		reset:
			_escape_nchars = 0;
			_escape_buffer[0] = '\0';
			continue; // next char
		}
		else
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
				_escape_buffer[0] = '\033';
				_escape_nchars = 1;
			}
			else if (c == '\b')
			{
				_current_pos--;
				_buffer[_current_pos] = (((uint16_t) this->_attr_byte) << 8) | ' ';
			}
			else
			{
				_buffer[_current_pos] = (((uint16_t) this->_attr_byte) << 8) | c;
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
