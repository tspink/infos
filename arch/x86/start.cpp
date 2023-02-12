/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/start.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/init.h>
#include <arch/x86/x86-arch.h>
#include <arch/x86/multiboot.h>
#include <arch/x86/early-screen.h>
#include <arch/x86/qemu-stream.h>
#include <arch/x86/context.h>
#include <arch/x86/acpi/acpi.h>
#include <infos/kernel/log.h>
#include <infos/kernel/kernel.h>
#include <infos/util/string.h>
#include <infos/util/map.h>
#include <infos/util/printf.h>

using namespace infos::arch::x86;
using namespace infos::arch::x86::acpi;
using namespace infos::kernel;
using namespace infos::util;

//static EarlyScreen early_screen;

// Create a QEMU stream for the syslog.
QEMUStream infos::arch::x86::qemu_stream;
ComponentLog infos::arch::x86::x86_log(syslog, "x86");

// Reference to the value of EBX at multiboot entry.
extern phys_addr_t __multiboot_ebx;

// Reference to the multiboot info structure.
struct multiboot_info *infos::arch::x86::multiboot_info_structure;

// In case a severe problem happened early on.
static void __attribute__((noreturn)) early_abort()
{
	for (;;) {
		asm volatile("hlt");
	}
}

// References to the C++ static initialisers.
extern "C" {
	extern void (*__init_array_start []) (void);
	extern void (*__init_array_end []) (void);
}

/**
 * Runs the static constructors.
 */
static void run_static_constructors()
{
	size_t size = __init_array_end - __init_array_start;

	for (unsigned int i = 0; i < size; i++) {
		(*__init_array_start[i])();
	}
}

/**
 * Removes the lower memory mapping, that identity maps physical memory.
 */
static void eliminate_lower_mapping()
{
	// Load the current page table base address
	uint64_t cr3;
	asm volatile("mov %%cr3, %%rax" : "=a"(cr3));
	
	// Eliminate the lower mapping from the PML4
	uint64_t *pml4 = (uint64_t *)(pa_to_kva(cr3));
	pml4[0] = 0;
	
	// Reload the page tables
	asm volatile("mov %%rax, %%cr3" :: "a"(cr3));
}

/**
 * Post-generic initialisation for the x86 architecture.
 */
static bool x86_init_bottom()
{
	x86_log.message(LogLevel::DEBUG, "Initialising and activating console");
	if (!console_init()) {
		syslog.message(LogLevel::ERROR, "Unable to initialise console");
		goto init_error;
	}
	
	if (!activate_console()) {
		syslog.message(LogLevel::ERROR, "Unable to activate console");
		goto init_error;
	}
	
	if (!devices_init()) {
		syslog.message(LogLevel::ERROR, "Unable to initialise platform devices");
		goto init_error;
	}

	return true;
	
init_error:
	return false;
}

// Zeroes out the BSS section -- necessary in case any BIOS data has clobbered the BSS area.
extern char _BSS_START, _BSS_END, _STACK2_END;
static void zero_bss()
{
	infos::util::bzero(&_BSS_START, (&_BSS_END - &_BSS_START));
}

extern "C" void __noreturn x86_init_top()
{
	// Zero-out the BSS section, so that uninitialised static/global variables are zero.
	zero_bss();
	
	// Fix-up the multiboot info structure BEFORE we eliminate the lower mapping
	multiboot_info_structure = (struct multiboot_info *)pa_to_kva(__multiboot_ebx);
	eliminate_lower_mapping();

	run_static_constructors();

	syslog.set_stream(qemu_stream);
	//syslog.set_stream(early_screen);
	syslog.colour(true);

	syslog.message(LogLevel::INFO, "-----------------------------------------------------------------");
	syslog.message(LogLevel::INFO, "Starting InfOS!");
	syslog.message(LogLevel::INFO, "-----------------------------------------------------------------");

	syslog.messagef(
		LogLevel::INFO,
		"Kernel command-line: %s",
		(const char *)(pa_to_kva((uint64_t)multiboot_info_structure->cmdline)));	
		
	sys.early_init((const char *)(pa_to_kva((uint64_t)multiboot_info_structure->cmdline)));
	
	x86_log.message(LogLevel::DEBUG, "Initialising platform");
	if (!x86arch.init()) {
		syslog.message(LogLevel::ERROR, "Unable to initialise the platform");
		goto init_error;
	}
	
	x86_log.message(LogLevel::DEBUG, "Initialising memory management");
	if (!mm_init()) {
		syslog.message(LogLevel::ERROR, "Unable to initialise the memory manager");
		goto init_error;
	}
	
	x86_log.message(LogLevel::DEBUG, "Initialising IRQs");
	if (!x86arch.init_irq()) {
		syslog.message(LogLevel::ERROR, "Unable to initialise IRQs");
		goto init_error;
	}
	
	mm_pf_init();
	
	x86_log.message(LogLevel::DEBUG, "Configuring platform with ACPI");
	if (!acpi_init()) {
		syslog.message(LogLevel::ERROR, "Unable to configure with ACPI");
		goto init_error;
	}

	x86_log.message(LogLevel::DEBUG, "Initialising CPU");
	if (!cpu_init()) {
		syslog.message(LogLevel::ERROR, "Unable to initialise the CPU");
		goto init_error;
	}
		
	x86_log.message(LogLevel::DEBUG, "Initialising boot modules");
	if (!modules_init()) {
		syslog.message(LogLevel::ERROR, "Unable to initialise boot modules");
		goto init_error;
	}
	
	x86_log.message(LogLevel::DEBUG, "Initialising timer");
	if (!timer_init()) {
		syslog.message(LogLevel::ERROR, "Unable to initialise timer");
		goto init_error;
	}
		
	x86_log.message(LogLevel::DEBUG, "Initialising scheduler");
	if (!sched_init()) {
		syslog.message(LogLevel::ERROR, "Unable to initialise scheduler");
		goto init_error;
	}
	
	// Start the system, and begin executing the second-half of the
	// arch specific initialisation.
	sys.start(x86_init_bottom);
	
init_error:
	early_abort();
}

extern "C" {
	void handle_unhandled_exception()
	{
		syslog.message(LogLevel::FATAL, "Unhandled Exception!");
	}
}
