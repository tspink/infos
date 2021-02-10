/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/start64.S
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
.code64
.section .startup.text64, "ax"

.align 16

.globl start64
.type start64, %function
start64:
    // Re-initialise segment registers
    mov $0x10, %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

    xor %eax, %eax
    mov %ax, %fs
    mov %ax, %gs

    // Switch the stack address to the high address space
    mov $_STACK_END, %rsp

    // Jump to the x86 initialisation entrypoint
    lea x86_init_top, %rax
    jmp *%rax
.size start64,.-start64
