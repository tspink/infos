/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/qemu-stream.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/qemu-stream.h>
#include <arch/x86/pio.h>

using namespace infos::arch::x86;

int QEMUStream::read(void* buffer, size_t size)
{
	return 0;
}

int QEMUStream::write(const void* buffer, size_t size)
{
	for (unsigned int i = 0; i < size; i++) {
		__outb(0xe9, ((char *)buffer)[i]);
	}
	
	return size;
}
