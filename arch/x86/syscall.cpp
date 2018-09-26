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
	//syslog.messagef(LogLevel::DEBUG, "USER SYSTEM CALL: thread=%p", Thread::current());
	
	X86Context *ctx = (X86Context *)Thread::current().context().native_context;
	
	sys.arch().enable_interrupts();
	Syscalls::Syscalls syscall = (Syscalls::Syscalls)ctx->rax;
	
	switch(syscall) {
	case Syscalls::SYS_OPEN:
		ctx->rax = Syscall::sys_open(ctx->rdi, ctx->rsi);
		break;

	case Syscalls::SYS_CLOSE:
		ctx->rax = Syscall::sys_close(ctx->rdi);
		break;
		
	case Syscalls::SYS_READ:
		ctx->rax = Syscall::sys_read(ctx->rdi, ctx->rsi, ctx->rdx);
		break;
		
	case Syscalls::SYS_WRITE:
		ctx->rax = Syscall::sys_write(ctx->rdi, ctx->rsi, ctx->rdx);
		break;
		
	case Syscalls::SYS_OPENDIR:
		ctx->rax = Syscall::sys_opendir(ctx->rdi, ctx->rsi);
		break;
		
	case Syscalls::SYS_READDIR:
		ctx->rax = Syscall::sys_readdir(ctx->rdi, ctx->rsi);
		break;
		
	case Syscalls::SYS_CLOSEDIR:
		ctx->rax = Syscall::sys_closedir(ctx->rdi);
		break;
		
	case Syscalls::SYS_EXIT:
		Syscall::sys_exit(ctx->rdi);
		break;
		
	case Syscalls::SYS_EXEC:
		ctx->rax = Syscall::sys_exec(ctx->rdi, ctx->rsi);
		break;
		
	case Syscalls::SYS_WAIT_PROC:
		ctx->rax = Syscall::sys_wait_proc(ctx->rdi);
		break;
		
	case Syscalls::SYS_CREATE_THREAD:
		ctx->rax = Syscall::sys_create_thread(ctx->rdi, ctx->rsi);
		break;

	case Syscalls::SYS_STOP_THREAD:
		ctx->rax = Syscall::sys_stop_thread(ctx->rdi);
		break;
		
	case Syscalls::SYS_JOIN_THREAD:
		ctx->rax = Syscall::sys_join_thread(ctx->rdi);
		break;
		
	case Syscalls::SYS_USLEEP:
		Syscall::sys_usleep(ctx->rdi);
		break;
		
	default:
		ctx->rax = -1;
		syslog.messagef(LogLevel::DEBUG, "UNHANDLED USER SYSTEM CALL: %lu", syscall);
		break;
	}
	
	sys.arch().disable_interrupts();
}

/**
 * Handle a system call that came from the kernel.
 */
void kernel_syscall_handler(const IRQ *irq, void *priv)
{
	Syscalls::Syscalls syscall = (Syscalls::Syscalls)Thread::current().context().native_context->rax;
	
	//syslog.messagef(LogLevel::DEBUG, "KERNEL SYSTEM CALL: rcx=%lx", current_context.rax);
	
	switch(syscall) {
	case Syscalls::SYS_YIELD:
		sys.scheduler().schedule();
		break;
		
	default:
		syslog.messagef(LogLevel::DEBUG, "UNHANDLED SYSTEM CALL %lu", syscall);
		break;
	}
}
