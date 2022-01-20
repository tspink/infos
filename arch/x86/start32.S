/* SPDX-License-Identifier: MIT */

#include "crflags.h"

/*
 * arch/x86/start32.S
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
.code32
.section .startup.text32, "ax"

.align 16

/**
 * 32-bit kernel entry point
 */
.globl start32
.type start32, %function
start32:
    // Initialise a stack for clearing the flags, and calling zero_pages32.  The maximum depth of this
    // stack should be 4 bytes, as we're in 32-bit mode at this point, and only use the stack for clearing
    // EFLAGS, and using the zero page helper function.
    mov $0x1000, %esp

    // Reset EFLAGS
    pushl $0
    popf

    // Load a null IDT
    lidt idtp

    // Prepare the initial page tables, with an identity map for
    // lower memory, and a corresponding mapping for the higher address space.

    // Prepare the PML4 @ 0x1000
    movl $0x1000, %ebx
    movl $5, %ecx
    call zero_pages32

    // Update cr3 with the address of the PML4
    mov %ebx, %cr3

    movl $0x2003, 0x000(%ebx)	// PDP @ 0x2000 (P, RW)
    movl $0x3003, 0xff8(%ebx)	// PDP @ 0x3000 (P, RW)

    // PDP @ 0x2000		(LOW VMEM)
    movl $0x2000, %ebx
    movl $0x4003, (%ebx)		// PD @ 0x4000 (P, RW)

    // PDP @ 0x3000		(HIGH VMEM)
    movl $0x3000, %ebx
    movl $0x5003, 0xff0(%ebx)	// PD @ 0x5000 (P, RW)
    movl $0x6003, 0xff8(%ebx)	// PD @ 0x6000 (P, RW)

    // PD @ 0x4000		(LOW VMEM)
    movl $0x4000, %ebx
    movl $0x000083, 0(%ebx)		// 2MB 1:1 Mapping

    // PD @ 0x5000		(HIGH VMEM 1G)
    movl $0x5000, %ebx

    // Map 2GB of physical memory into the upper virtual address space

    mov $0x83, %eax
    xor %ecx, %ecx

1:
    movl %eax, (%ebx, %ecx, 8)
    add $0x200000, %eax
    inc %ecx
    cmp $0x200, %ecx
    jne 1b

    xor %ecx, %ecx

    // PD @ 0x6000		(HIGH VMEM 2G)
    movl $0x6000, %ebx

1:
    movl %eax, (%ebx, %ecx, 8)
    add $0x200000, %eax
    inc %ecx
    cmp $0x200, %ecx
    jne 1b

    // Disable the PIC
    mov $0xff, %al
    out %al, $0xa1
    out %al, $0x21

    // CR4 :=
    mov $0x6b0, %eax
    mov %eax, %cr4

    // CR4
//    mov $(CR4_PSE | CR4_PAE | CR4_PGE | CR4_OSFXSR | CR4_OSXMMEXCPT), %edi

    // Check for XSAVE CPUID
//    mov $1, %eax
//    cpuid
//    bt $26, %ecx
//    jnc 1f

//    or $(CR4_OSXSAVE), %edi

    // EFER :=
    mov $0xC0000080, %ecx
    rdmsr
    or $0x000000901, %eax

//1:
//    mov %edi, %cr4

    // EFER
//    mov $(0xC0000080), %ecx
//    rdmsr
//    or $(EFER_SCE | EFER_LME | EFER_NXE), %eax
    wrmsr

    // CR0 := PG, PE, MP, EM, WP
    mov $0x80010007, %eax

    // CR0
//    mov $(CR0_PG | CR0_PE | CR0_MP | CR0_WP), %eax
    mov %eax, %cr0

    // XCR0
//    mov $1, %eax
//    cpuid
//    bt $27, %ecx
//    jnc 1f

//    xor %ecx, %ecx
//    xor %eax, %eax
//    xor %edx, %edx
//    xgetbv
//    or $7, %eax
//    xsetbv

//1:

    // Initialise the temporary GDT
    lgdt gdpt

    // Long jump to 64-bit code
    ljmp $0x8, $start64

/**
 * Quickly zeroes out a number of 4k pages.
 * @ebx: Base address of page.
 * @ecx: Number of pages.
 *
 * Clobbers: %edi, %eax, %ecx & DF
 */
.align 16
.type zero_pages32, %function
zero_pages32:
    mov %ebx, %edi
    xor %eax, %eax
    cld
    shl $10, %ecx
    repz stosl
    ret

.section .startup.data, "a"

/* Null IDT pointer */
.align 4
idtp:
    .word 0
    .long 0

/* Temporary GDT */
.align 4
gdt:
    .quad 0x0000000000000000
    .quad 0x00209A0000000000
    .quad 0x0000920000000000
gdt_end:

/* Temporary GDT pointer */
.align 4
gdpt:
    .word (gdt_end - gdt)
    .long gdt
