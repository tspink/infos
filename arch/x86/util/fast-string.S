/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/util/fast-string.S
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
.text

.align 16
.globl memcpy
memcpy:
	mov %rdi, %rax

	cld

	mov %rdx, %rcx
	shr $3, %rcx
	
	rep movsq %ds:(%rsi), %es:(%rdi)

	mov %rdx, %rcx
	and $7, %rcx

	rep movsb %ds:(%rsi), %es:(%rdi)
	
	ret

.align 16
.globl memset
memset:
	test %rsi, %rsi
	jz 1f

	cld
	mov %esi, %eax
	mov %rdx, %rcx
	jmp 2f

1:
	mov %rdx, %rsi

.align 16
.globl bzero
bzero:
	cld

	mov %rsi, %rcx
	shr $3, %rcx

	xor %eax, %eax
	
	rep stosq %rax, %es:(%rdi)

	movq %rsi, %rcx
	and $3, %rcx

.align 16
2:
	rep stosb %al, %es:(%rdi)

	mov %rdi, %rax
	ret

.align 16
.globl pzero
pzero:
	cld
	mov $512, %ecx
	xor %eax, %eax
	rep stosq %rax, %es:(%rdi)
	ret

.align 16
.globl pnzero
pnzero:
	cld
	mov %esi, %ecx
	shl $9, %ecx
	xor %eax, %eax
	rep stosq %rax, %es:(%rdi)
	ret
