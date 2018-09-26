/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/multiboot.S
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#define STACK_SIZE				4096

#define MULTIBOOT_HEADER_MAGIC  0x1BADB002
#define MULTIBOOT_HEADER_FLAGS  0x00000003

.section .multiboot.header, "a"

// Multiboot Header Section, contains the magic number, a series of flags
// and the checksum.

.align 4

.type multiboot_header, %object
multiboot_header:
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
.size multiboot_header,.-multiboot_header

.code32

.section .multiboot.text, "ax"

// Multiboot entry point

.align 16

.globl multiboot_start
.type multiboot_start, %function
multiboot_start:
	// Disable interrupts
	cli

	// Make sure the incoming magic number is correct
	cmp $0x2BADB002, %eax
	jne panic

	lea __multiboot_ebx, %ecx
	mov %ebx, (%ecx)

	jmp start32

panic:
	pause
	hlt
	jmp panic

.section .startup.data, "a"
.globl __multiboot_ebx
__multiboot_ebx:	.quad 0
