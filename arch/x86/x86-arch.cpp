/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/x86-arch.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/x86-arch.h>
#include <arch/x86/init.h>
#include <arch/x86/cpu.h>
#include <arch/x86/cpuid.h>
#include <arch/x86/irq.h>
#include <arch/x86/dt.h>
#include <arch/x86/msr.h>
#include <arch/x86/context.h>
#include <infos/kernel/log.h>
#include <infos/kernel/thread.h>
#include <infos/kernel/process.h>
#include <infos/util/string.h>

extern "C" {
	infos::kernel::Thread *current_thread;
}

using namespace infos::arch;
using namespace infos::arch::x86;
using namespace infos::kernel;
using namespace infos::util;

X86Arch infos::arch::x86::x86arch;
Arch& infos::arch::sys_arch = x86arch;
X86CPU bsp;

extern "C" void __syscall_trap(void);
extern void kernel_syscall_handler(const IRQ *irq, void *priv);
extern void user_syscall_handler(const IRQ *irq, void *priv);

static void general_protection_fault(const IRQ *irq, void *priv)
{
	x86_log.message(LogLevel::FATAL, "EXCEPTION: General Protection Fault");
	sys.arch().dump_current_context();
	arch_abort();
}

static void trap_interrupt(const IRQ *irq, void *priv)
{
	x86_log.message(LogLevel::FATAL, "EXCEPTION: TRAP");
	sys.arch().dump_current_context();
	arch_abort();
}

X86Arch::X86Arch()
{
	_cpus[0] = &bsp;
}

bool X86Arch::init()
{
	x86_log.message(LogLevel::INFO, "Initilising GDT");
	if (!gdt.init()) {
		return false;
	}

	x86_log.message(LogLevel::INFO, "Initilising IDT");
	if (!idt.init()) {
		return false;
	}

	x86_log.message(LogLevel::INFO, "Initilising TSS");
	if (!tss.init(0x28)) {
		return false;
	}

	uint64_t rsp;
	asm volatile("mov %%rsp, %0" : "=r"(rsp));

	x86_log.messagef(LogLevel::DEBUG, "GDTR = %p, IDTR = %p, TR = %p, RSP = %p", gdt.get_ptr(), idt.get_ptr(), tss.get_sel(), rsp);

	__wrmsr(MSR_STAR, 0x18000800000000ULL);				// CS Bases for User-Mode/Kernel-Mode
	__wrmsr(MSR_LSTAR, (uint64_t)__syscall_trap);		// RIP for syscall entry
	__wrmsr(MSR_SFMASK, (1 << 9));

	auto feat = cpuid_get_features();
	if (!(feat.rcx & (uint64_t)CPUIDFeatures::OSXSAVE)) {
		syslog.message(LogLevel::WARNING, "XSAVE not supported");
	} else {
		syslog.message(LogLevel::INFO, "XSAVE enabled");
	}

	return true;
}

bool X86Arch::init_irq()
{
	if (!_irq_manager.init()) {
		return false;
	}

	_irq_manager.install_exception_handler(IRQ_GPF, general_protection_fault, NULL);
	_irq_manager.install_exception_handler(IRQ_TRAP, trap_interrupt, NULL);
	_irq_manager.install_software_handler(IRQ_KERNEL_SYSCALL, kernel_syscall_handler, NULL);
	_irq_manager.install_software_handler(IRQ_USER_SYSCALL, user_syscall_handler, NULL);

	return true;
}

void X86Arch::dump_current_context() const
{
	dump_thread_context(get_current_thread().context());
}

void X86Arch::dump_thread_context(const kernel::ThreadContext& context) const
{
	syslog.messagef(LogLevel::DEBUG, "*** THREAD CONTEXT %p (NC=%p)", &context, context.native_context);
	dump_native_context(*context.native_context);
}

void X86Arch::dump_stack(const kernel::ThreadContext& context) const
{

}

void X86Arch::dump_native_context(const X86Context& native_context) const
{
	syslog.messagef(LogLevel::DEBUG, "rax=%016lx, rbx=%016lx, rcx=%016lx, rdx=%016lx",
			native_context.rax,
			native_context.rbx,
			native_context.rcx,
			native_context.rdx);

	syslog.messagef(LogLevel::DEBUG, "rsi=%016lx, rdi=%016lx, rsp=%016lx, rbp=%016lx",
			native_context.rsi,
			native_context.rdi,
			native_context.rsp,
			native_context.rbp);

	syslog.messagef(LogLevel::DEBUG, " r8=%016lx,  r9=%016lx, r10=%016lx, r11=%016lx",
			native_context.r8,
			native_context.r9,
			native_context.r10,
			native_context.r11);

	syslog.messagef(LogLevel::DEBUG, "r12=%016lx, r13=%016lx, r14=%016lx, r15=%016lx",
			native_context.r12,
			native_context.r13,
			native_context.r14,
			native_context.r15);

	syslog.messagef(LogLevel::DEBUG, "rip=%016lx, cs=%04lx, ss=%04lx, rflags=%016lx, extra=%lx",
			native_context.rip,
			native_context.cs,
			native_context.ss,
			native_context.rflags,
			native_context.extra);

	syslog.messagef(LogLevel::DEBUG, "prev=%p", native_context.previous_context);
}

void X86Arch::invoke_kernel_syscall(int nr)
{
	asm volatile("int $0x80" :: "a"((uint64_t)nr));
}

infos::kernel::Thread& X86Arch::get_current_thread() const
{
	return *current_thread;
}

void X86Arch::set_current_thread(kernel::Thread& thread)
{
	asm volatile("mov %0, %%cr3" :: "r"(thread.owner().vma().pgt_base()) : "memory");

	tss.set_kernel_stack(thread.context().kernel_stack);
	current_thread = &thread;
}

IRQ *X86Arch::request_irq()
{

	return NULL; //_irq_manager.request_irq();
}

extern "C" {
	void *get_current_thread_context()
	{
		if (!current_thread) return NULL;
		//assert(current_thread);
		return &current_thread->context();
	}

	void __debug_save_context()
	{
		assert(current_thread);
		syslog.messagef(LogLevel::DEBUG, "Save Context %p %p", current_thread, current_thread->context());
	}

	void __debug_restore_context()
	{
		assert(current_thread);
		syslog.messagef(LogLevel::DEBUG, "Restore Context %p %p", current_thread, current_thread->context());
	}
}
