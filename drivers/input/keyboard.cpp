/* SPDX-License-Identifier: MIT */

/*
 * drivers/input/keyboard.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/input/keyboard.h>
#include <infos/kernel/device-manager.h>
#include <infos/kernel/irq.h>
#include <infos/kernel/log.h>
#include <infos/drivers/irq/ioapic.h>
#include <infos/drivers/irq/lapic.h>
#include <arch/x86/pio.h>

using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::drivers::input;
using namespace infos::drivers::irq;
using namespace infos::arch::x86;

static Keys::Keys scancode_map[] = {
	Keys::NO_KEY, Keys::KEY_ESCAPE,
	Keys::KEY_1, Keys::KEY_2, Keys::KEY_3, Keys::KEY_4, Keys::KEY_5, Keys::KEY_6,
	Keys::KEY_7, Keys::KEY_8, Keys::KEY_9, Keys::KEY_0,
	
	Keys::KEY_HYPHEN, Keys::KEY_EQUALS, Keys::KEY_BACKSPACE, Keys::KEY_TAB,
	
	Keys::KEY_Q, Keys::KEY_W, Keys::KEY_E, Keys::KEY_R, Keys::KEY_T, Keys::KEY_Y,
	Keys::KEY_U, Keys::KEY_I, Keys::KEY_O, Keys::KEY_P, Keys::KEY_LSQBRACKET, Keys::KEY_RSQBRACKET,
	
	Keys::KEY_RETURN,
	Keys::KEY_LCTRL,
	
	Keys::KEY_A, Keys::KEY_S, Keys::KEY_D, Keys::KEY_F, Keys::KEY_G, Keys::KEY_H,
	Keys::KEY_J, Keys::KEY_K, Keys::KEY_L, Keys::KEY_SEMICOLON, Keys::KEY_APOSTROPHE, Keys::KEY_BACKTICK,
	
	Keys::KEY_LSHIFT,
	Keys::KEY_BACKSLASH, Keys::KEY_Z, Keys::KEY_X, Keys::KEY_C, Keys::KEY_V, Keys::KEY_B,
	Keys::KEY_N, Keys::KEY_M, Keys::KEY_COMMA, Keys::KEY_DOT, Keys::KEY_FORSLASH,

	Keys::KEY_RSHIFT,
	Keys::KEY_ASTERISK,
	Keys::KEY_LALT,
	Keys::KEY_SPACE,
	Keys::KEY_CAPSLOCK,
	
	Keys::KEY_F1,
	Keys::KEY_F2,
	Keys::KEY_F3,
	Keys::KEY_F4,
	Keys::KEY_F5,
	Keys::KEY_F6,
	Keys::KEY_F7,
	Keys::KEY_F8,
	Keys::KEY_F9,
	Keys::KEY_F10,
};

const DeviceClass Keyboard::KeyboardDeviceClass(Device::RootDeviceClass, "kbd");

Keyboard::Keyboard() : _irq(NULL), _sink(NULL)
{

}

/**
 * Initialises the keyboard device.
 * @param dm The device manager owning this keyboard device.
 * @return Returns TRUE if the device was successfully initialised,
 * or FALSE otherwise.
 */
bool Keyboard::init(kernel::DeviceManager& dm)
{
	// Find the LAPIC.
	LAPIC *lapic;
	if (!dm.try_get_device_by_class(LAPIC::LAPICDeviceClass, lapic)) {
		return false;
	}
	
	// Find the IOAPIC.
	IOAPIC *ioapic;
	if (!dm.try_get_device_by_class(IOAPIC::IOAPICDeviceClass, ioapic)) {
		return false;
	}
	
	// Hook-up IRQ 1 on the IOAPIC to the IRQ handler object, and register
	// the IRQ callback function.
	_irq = ioapic->request_physical_irq(lapic, 1);
	_irq->attach(keyboard_irq_handler, this);
	
	return true;
}

/**
 * Handle a particular keyboard event.
 * @param scancode The scancode associated with the event.
 */
void Keyboard::handle_key_event(int8_t scancode)
{
	// No sink?  Nothing to do.
	if (_sink == NULL) return;
	
	// Determine whether or not the scancode corresponds to a push
	// or release event.
	if (scancode & 0x80) {
		// Convert the scancode, and submit to the sink's key_up handler.
		_sink->key_up(scancode_to_key(scancode));
	} else {
		// Convert the scancode, and submit to the sink's key_down handler.
		_sink->key_down(scancode_to_key(scancode));
	}
}

/**
 * Takes in a raw scancode, and converts it into a managed key.  This discards
 * push/release information.
 * @param scancode The scancode to convert.
 * @return Returns a managed key representation.
 */
Keys::Keys Keyboard::scancode_to_key(int8_t scancode)
{
	uint8_t key_scancode = ((uint8_t)scancode) & ~0x80;
	
	if (key_scancode > ARRAY_SIZE(scancode_map)) {
		return Keys::UNKNOWN_KEY;
	}
	
	return scancode_map[key_scancode];
}

/**
 * The interrupt handler for the keyboard interrupt.
 * @param irq A pointer to the IRQ object representing the IRQ that was signalled.
 * @param priv A pointer to private data associated with the IRQ.
 */
void Keyboard::keyboard_irq_handler(const kernel::IRQ *irq, void *priv)
{
	// Read the scancode in from the keyboard buffer.
	int8_t scancode = (int8_t)__inb(0x60);
	
	// Post the event into the keyboard device.
	((Keyboard *)priv)->handle_key_event(scancode);
}
