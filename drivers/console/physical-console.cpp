/* SPDX-License-Identifier: MIT */

/*
 * drivers/console/physical-console.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/console/physical-console.h>
#include <infos/drivers/console/virtual-console.h>
#include <infos/drivers/input/keyboard.h>

using namespace infos::drivers;
using namespace infos::drivers::console;

const DeviceClass PhysicalConsole::PhysicalConsoleDeviceClass(ConsoleDeviceClass, "pc");

PhysicalConsole::PhysicalConsole() : _kbd(NULL), _current_vc(NULL), _alt_pressed(false)
{

}

void PhysicalConsole::key_up(input::Keys::Keys key)
{
	switch (key) {
	case input::Keys::KEY_LALT:
	case input::Keys::KEY_RALT:
		_alt_pressed = false;	// Fallthrough
		
	default:
		if (_current_vc) {
			_current_vc->key_up(key);
		}
		break;
	}
}

void PhysicalConsole::key_down(input::Keys::Keys key)
{
	// Yes, I know this switch statement looks weird, but
	// its to reduce code duplication.  If ALT is not pressed,
	// then F1 & F2 need to be passed through to the underlying
	// VC.  Otherwise, the function keys are captured.
	switch (key) {
	case input::Keys::KEY_F1:
		if (_alt_pressed) {
			set_current_vc(*_vcs.at(0));
			return;
		}
		break;
		
	case input::Keys::KEY_F2:
		if (_alt_pressed) {
			set_current_vc(*_vcs.at(1));
			return;
		}
		break;
		
	case input::Keys::KEY_LALT:
	case input::Keys::KEY_RALT:
		_alt_pressed = true;
		break;
		
	default:
		break;
	}
		
	if (_current_vc) {
		_current_vc->key_down(key);
	}
}

void PhysicalConsole::add_virtual_console(VirtualConsole& vc)
{
	_vcs.append(&vc);
	
	if (_vcs.count() == 1) {
		set_current_vc(vc);
	}
}
