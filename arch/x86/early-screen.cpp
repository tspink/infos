/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/early-screen.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/early-screen.h>
#include <arch/x86/pio.h>
#include <infos/util/string.h>

using namespace infos::arch::x86;
using namespace infos::util;

/**
 * Constructs an EarlyScreen object.  The _video_ram is set to the known
 * framebuffer address of 0xb8000
 */
EarlyScreen::EarlyScreen() : _video_ram((volatile uint16_t *)(pa_to_kva(0xb8000)))
{
}

/**
 * Initialises the early screen.
 */
void EarlyScreen::init()
{
	uint16_t position;
	
	// Figure out what the current cursor position of the VGA framebuffer is.
	
	// Read the 16-bit position value.
	__outb(0x3d4, 0xf);
	position = __inb(0x3d5);

	__outb(0x3d4, 0xe);
	position |= __inb(0x3d5) << 8;
	
	// Convert it to X and Y co-ordinates.
	_current_x = position % _width;
	_current_y = position / _width;
}

/**
 * Clears the framebuffer, and resets the cursor position.
 */
void EarlyScreen::clear()
{
	_current_x = 0;
	_current_y = 0;
	update_cursor();

	unsigned int x, y;
	for (y = 0; y < _height; y++) {
		for (x = 0; x < _width; x++) {
			_video_ram[x + (y * _width)] = 0;
		}
	}	
}

/**
 * Places a character on the screen at the current cursor position, and advances
 * the cursor.
 * @param attr The attributes for the character (e.g. colour information)
 * @param c The character to display.
 */
void EarlyScreen::putch(int attr, char c)
{
	if (c == '\r') {
		// If the character is a "carriage-return", then reset the current X position
		// to the beginning of the line.
		_current_x = 0;
	} else if (c == '\n') {
		// If the character is a "new-line", then reset the current X position,
		// advance the current Y position, and possibly scroll the screen.
		_current_x = 0;
		if (++_current_y == _height) {
			_current_y = _height - 1;
			scroll();
		}
	} else {
		// Insert the character and its attributes into the framebuffer memory.
		_video_ram[_current_x + (_current_y * _width)] = ((attr & 0xff) << 8) | c;

		// Advance the X position, but if we go over the width of the screen,
		// move to the next line (possibly also scrolling the screen).
		if (++_current_x == _width) {
			_current_x = 0;
			if (++_current_y == _height) {
				_current_y = _height - 1;
				scroll();
			}
		}
	}
	
	// Instruct the framebuffer to update the hardware cursor location.
	update_cursor();
}

/**
 * Write a string out to the screen.
 * @param attr The attribute to apply to each character of the string.
 * @param str The string to write out.
 */
void EarlyScreen::putstr(int attr, const char *str)
{
	// Whilst there are characters in the string...
	while (*str) {
		// Put the character to the string, and advance the
		// string pointer.
		putch(attr, *str++);
	}
}

/**
 * Scrolls the screen by one line, by pushing everying up.
 */
void EarlyScreen::scroll()
{
	// Copy the block of framebuffer memory that is to be moved on top of itself, to
	// scroll the output.
	memcpy((void *)_video_ram, (const void *)&_video_ram[_width], (_width * (_height - 1)) * 2);
	
	// Zero out the last line of the screen.
	for (unsigned int x = 0; x < _width; x++) {
		_video_ram[x + ((_height - 1) * _width)] = 0;
	}
}

/**
 * Instructs the framebuffer device to update the location of the hardware cursor.
 */
void EarlyScreen::update_cursor()
{
	// Calculate the linear position of the cursor.
	uint16_t position = _current_x + (_current_y * _width);
	
	// Index register CP:LOW
	__outb(0x3d4, 0xf);
	__outb(0x3d5, position);

	// Index register CP:HI
	__outb(0x3d4, 0xe);
	__outb(0x3d5, (position >> 8));
}

/**
 * Writes a buffer out to the screen.
 * @param buffer A pointer to the buffer containing the data to be written.
 * @param size The size of the buffer.
 * @return Returns the number of bytes written.
 */
int EarlyScreen::write(const void *buffer, size_t size)
{
	// Convert the buffer to a character pointer, so we can iterate over it
	// easily.
	const char *p = (const char *)buffer;
	
	// For the size of the buffer, write out each character sequentially.
	while (size-- > 0) {
		// Boring white-on-black attribute.
		putch(0x07, *p++);
	}
	
	// This function 'just worked'.
	return size;
}

/**
 * Reads from the early screen device.  This is NOT supported.
 * @param buffer The buffer to read the data into.
 * @param size The size of the buffer.
 * @return Returns the number of characters read into the buffer.
 */
int EarlyScreen::read(void *buffer, size_t size)
{
	// This operation is NOT supported.
	return 0;
}
