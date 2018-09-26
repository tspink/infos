/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/assert.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/define.h>
#include <infos/kernel/log.h>

/**
 * Called when a particular assertion has failed, in order to print out debugging
 * information and halt the system.
 * @param filename The file that contains the assertion definition.
 * @param lineno The line on which the assertion is defined.
 * @param expression The expression that failed to be true.
 */
void __assertion_failure(const char *filename, int lineno, const char *expression)
{
	// Disable interrupts.
	asm volatile("cli");
	
	// Print out a separator, so that the assertion can be easily distinguished
	// on the console.
	infos::kernel::syslog.message(infos::kernel::LogLevel::FATAL, "*****");
	
	// Now, print out as much information as possible about the failure.
	infos::kernel::syslog.messagef(
			infos::kernel::LogLevel::FATAL,
			"ASSERTION FAILURE: %s:%d",
			filename,
			lineno);
	
	infos::kernel::syslog.messagef(
			infos::kernel::LogLevel::FATAL,
			"%s",
			expression);
	
	infos::kernel::syslog.message(infos::kernel::LogLevel::FATAL, "*****");
	
	// Attempt to walk the stack, by following the frame-pointer chain.  If something
	// terrible has happened, the stack may be wrecked.  But, we should at least
	// try to print out some helpful information.
	uint64_t rbp;
	asm volatile("mov %%rbp, %0" : "=r"(rbp));	// Read the base-pointer
	
	// Walk the stack frames by following the stacked base-pointers, until there
	// is no more, or the depth reaches 100 (whichever comes first).
	unsigned int frame_idx = 0, count = 0;
	while (rbp && count++ < 100) {
		uint64_t *stack = (uint64_t *)rbp;
		infos::kernel::syslog.messagef(infos::kernel::LogLevel::DEBUG, "  %u: %lx", frame_idx++, stack[1]);
		rbp = stack[0];
	}
	
	// TODO: It would be nice to also print out symbol information, but that would require initialising
	// a symbol table at some point.
	
	arch_abort();
}
