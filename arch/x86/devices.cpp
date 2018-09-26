/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/devices.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/init.h>
#include <arch/x86/multiboot.h>
#include <arch/x86/msr.h>
#include <arch/x86/cpuid.h>
#include <arch/x86/acpi/acpi.h>

#include <infos/kernel/log.h>
#include <infos/kernel/kernel.h>

#include <infos/drivers/console/virtual-console.h>
#include <infos/drivers/terminal/terminal.h>
#include <infos/drivers/video/vga-console.h>
#include <infos/drivers/input/keyboard.h>
#include <infos/drivers/timer/lapic-timer.h>
#include <infos/drivers/timer/pit.h>
#include <infos/drivers/pci/pci-bus.h>
#include <infos/drivers/irq/lapic.h>
#include <infos/drivers/irq/ioapic.h>

#include <infos/util/cmdline.h>
#include <infos/util/string.h>

// Define a command-line argument that lets us redirect the kernel
// syslog to the serial port.
static bool syslog_to_serial = false;

RegisterCmdLineArgument(SysLogDestination, "syslog") {
	if (infos::util::strncmp(value, "serial", 6) == 0) {
		syslog_to_serial = true;
	} else {
		syslog_to_serial = false;
	}
}

using namespace infos::arch::x86;
using namespace infos::drivers;
using namespace infos::drivers::console;
using namespace infos::drivers::terminal;
using namespace infos::drivers::input;
using namespace infos::drivers::pci;
using namespace infos::drivers::video;
using namespace infos::drivers::timer;
using namespace infos::drivers::irq;
using namespace infos::kernel;

/**
 * Initialise the main system timer.
 * @return Returns TRUE if the timer was successfully initialised, or FALSE otherwise.
 */
bool infos::arch::x86::timer_init()
{
	// Read the CPU feature set.
	CPUIDFeatures::CPUIDFeatures features = cpuid_get_features();
	
	// The timer requires the APIC, so check that it is present.
	if (!(features.rdx & CPUIDFeatures::APIC)) {
		syslog.message(LogLevel::ERROR, "APIC not present");
		return false;
	}
	
	// Determine the LAPIC base address.
	uint64_t lapic_base = __rdmsr(MSR_APIC_BASE) & ~0xfff;
	if (!lapic_base) {
		syslog.message(LogLevel::ERROR, "Invalid LAPIC base address");
		return false;
	}
	
	// Determine the IOAPIC base address.
	uint32_t ioapic_base = infos::arch::x86::acpi::acpi_get_ioapic_base();
	
	// Print out some information about the memory-mapped location of these
	// structures.
	syslog.messagef(LogLevel::DEBUG, "LAPIC base=%lx, IOAPIC base=%lx", lapic_base, ioapic_base);
	
	// Create and register an LAPIC object.
	LAPIC *lapic = new LAPIC(pa_to_vpa(lapic_base));
	if (!sys.device_manager().register_device(*lapic))
		return false;

	// Create and register an IOAPIC object.
	IOAPIC *ioapic = new IOAPIC(pa_to_vpa(ioapic_base));
	if (!sys.device_manager().register_device(*ioapic))
		return false;

	// Create and register a PIT (Programmable Interrupt Timer).  This isn't used as
	// the system timer, but rather as a reference timer for calibrating the LAPIC
	// timer.
	PIT *pit = new PIT();
	if (!sys.device_manager().register_device(*pit))
		return false;
	
	// Finally, create a register the LAPIC timer.  The LAPIC timer will
	// calibrate itself as part of its initialisation, and so the PIT
	// must be registered beforehand.
	LAPICTimer *lapic_timer = new LAPICTimer();
	if (!sys.device_manager().register_device(*lapic_timer))
		return false;
	
	// Set the timer to be periodic, with a period of 10ms, and start
	// the timer.
	lapic_timer->init_periodic((lapic_timer->frequency() >> 4) / 100);
	lapic_timer->start();
	
	return true;
}

/**
 * Initialises the devices necessary for providing a console.  This routine creates
 * and registers the devices, but does not connect everything together.  That happens
 * in activate_console().
 * @return Returns TRUE if the devices were successfully created and registered.
 */
bool infos::arch::x86::console_init()
{
	// Create a new VGA console device (which will be the /physical/ console), and
	// register it with the system.
	if (!sys.device_manager().register_device(*new VGAConsoleDevice(pa_to_vpa(0xb8000))))
		return false;
	
	// Create and register a keyboard device.
	if (!sys.device_manager().register_device(*new Keyboard()))
		return false;

	// Create a terminal that will be associated with the virtual console.
	Terminal *tty0 = new Terminal();	
	if (!sys.device_manager().register_device(*tty0))
		return false;

	// Add an alias to the TTY device called 'console'.
	if (!sys.device_manager().add_device_alias("console", *tty0))
		return false;
	
	// Create another terminal for a second virtual console.
	if (!sys.device_manager().register_device(*new Terminal()))
		return false;
	
	// Create and register two virtual console devices.
	if (!sys.device_manager().register_device(*new VirtualConsole()))
		return false;

	if (!sys.device_manager().register_device(*new VirtualConsole()))
		return false;
	
	return true;
}

/**
 * Fixes up the appropriate console devices and
 * switches the console on, possibly redirecting syslog, if configured to do so.
 * @return Returns TRUE if the console was successfully activated, or FALSE otherwise.
 */
bool infos::arch::x86::activate_console()
{
	// Lookup the virtual and physical console devices.
	PhysicalConsole *pc0;
	if (!sys.device_manager().try_get_device_by_class(console::PhysicalConsole::PhysicalConsoleDeviceClass, pc0)) return false;

	VirtualConsole *vc0, *vc1;
	if (!sys.device_manager().try_get_device_by_name("vc0", vc0)) return false;
	if (!sys.device_manager().try_get_device_by_name("vc1", vc1)) return false;
	
	// Lookup the keyboard device.
	Keyboard *kbd0;
	if (!sys.device_manager().try_get_device_by_class(input::Keyboard::KeyboardDeviceClass, kbd0)) return false;
	
	// Lookup the terminals that will be attached to the virtual consoles.
	Terminal *tty0, *tty1;
	if (!sys.device_manager().try_get_device_by_name("tty0", tty0)) return false;
	if (!sys.device_manager().try_get_device_by_name("tty1", tty1)) return false;
	
	// Attach the terminals to the virtual consoles.
	vc0->attach_terminal(tty0);
	vc1->attach_terminal(tty1);
	
	// Initialise the physical console
	kbd0->attach_sink(*pc0);
	pc0->add_virtual_console(*vc0);
	pc0->add_virtual_console(*vc1);
	
	// If syslog is not being redirected to the serial port, switch the
	// syslog output stream to the root terminal.
	if (!syslog_to_serial) {
		syslog.colour(false);
		syslog.set_stream(*tty0);
	}
	
	// Everything worked, so return true.
	return true;
}

/**
 * Initialises main system devices.
 * @return Returns TRUE if the devices were successfully initialised, or FALSE otherwise.
 */
bool infos::arch::x86::devices_init()
{
	// Create a new PCI bus object, with bus ID 0 and probe the bus for devices.
	PCIBus *bus = new PCIBus(0);
	return bus->probe(sys.device_manager());
}
