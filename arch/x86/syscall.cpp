/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/syscall.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/init.h>
#include <arch/x86/context.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/sched.h>
#include <infos/kernel/thread.h>
#include <infos/kernel/process.h>
#include <infos/kernel/log.h>
#include <infos/kernel/syscall.h>
#include <infos/kernel/object.h>
#include <infos/fs/vfs.h>
#include <infos/fs/file.h>
#include <arch/arch.h>

using namespace infos::kernel;
using namespace infos::fs;

/**
 * Handle a system call that came from user-space.
 */
void user_syscall_handler(const IRQ *irq, void *priv)
{
//	syslog.messagef(LogLevel::DEBUG, "USER SYSTEM CALL: thread=%p", Thread::current());

	X86Context *ctx = (X86Context *) Thread::current().context().native_context;

	sys.arch().enable_interrupts();

	ctx->rax = sys.syscalls().InvokeSyscall(ctx->rax, ctx->rdi, ctx->rsi, ctx->rdx, ctx->rcx, ctx->r8, ctx->r9);

	sys.arch().disable_interrupts();
}

/**
 * Handle a system call that came from the kernel.
 */
void kernel_syscall_handler(const IRQ *irq, void *priv)
{
//	syslog.messagef(LogLevel::DEBUG, "KERNEL SYSTEM CALL: rcx=%lx", current_context.rax);

	int syscall = Thread::current().context().native_context->rax;
	switch (syscall) {
	case 1:
		sys.scheduler().schedule();
		break;

	default:
		syslog.messagef(LogLevel::DEBUG, "UNHANDLED SYSTEM CALL %lu", syscall);
		break;
	}
}
