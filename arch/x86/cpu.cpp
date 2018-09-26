/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/cpu.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/init.h>
#include <arch/x86/cpu.h>
#include <arch/x86/msr.h>
#include <arch/x86/x86-arch.h>

using namespace infos::kernel;
using namespace infos::arch;
using namespace infos::arch::x86;

/**
 * Initialises the CPU.
 * @return Returns TRUE if the CPU was successfully initialised, or FALSE otherwise.
 */
bool infos::arch::x86::cpu_init()
{
	// This doesn't do anything, because we currently only support
	// a uniprocessor machine.  In the future, this would involve getting the CPU
	// online and configured.
	return true;
}

/**
 * Constructs a new X86CPU object.
 */
X86CPU::X86CPU()
{

}
