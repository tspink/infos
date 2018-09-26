/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/abort.S
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
.text
.global arch_abort
.type arch_abort, %function

.align 16

/*
 * Spins in a loop forever, with interrupts disabled.
 */
arch_abort:
	cli

1:
	hlt
	jmp 1b
