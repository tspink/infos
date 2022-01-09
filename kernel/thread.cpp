/* SPDX-License-Identifier: MIT */

/*
 * kernel/thread.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/thread.h>
#include <infos/kernel/process.h>
#include <infos/kernel/kernel.h>
#include <infos/mm/mm.h>
#include <infos/mm/page-allocator.h>
#include <infos/kernel/log.h>
#include <infos/util/string.h>
#include <arch/arch.h>

using namespace infos::kernel;
using namespace infos::util;

#define KERNEL_STACK_ORDER		1
#define KERNEL_STACK_SIZE		((1 << KERNEL_STACK_ORDER) * __page_size)

/**
 * Constructs a new thread object.
 */
Thread::Thread(Process& owner, ThreadPrivilege::ThreadPrivilege privilege, thread_proc_t entry_point,
               SchedulingEntityPriority::SchedulingEntityPriority priority, const util::String& name)
	: SchedulingEntity(priority, name),
	    _owner(owner),
		_privilege(privilege),
		_entry_point(entry_point),
		_current_entry_argument(0),
		_name(name)
{
	// Clear out the thread context.
	bzero(&_context, sizeof(_context));

	// Allocate the kernel stack for this thread.
	auto kernel_stack_pgd = owner.vma().allocate_phys(KERNEL_STACK_ORDER);
	assert(kernel_stack_pgd);

	// Record the base address of the kernel stack.  Stacks grow down, so add on the size of the stack
	// to the base address to create the starting address.
	_context.kernel_stack = (uintptr_t)sys.mm().pgalloc().pgd_to_vpa(kernel_stack_pgd);
	_context.kernel_stack += KERNEL_STACK_SIZE;

	// TODO: Check for XSAVE
	/*auto xsave_area_pgd = owner.vma().allocate_phys(0);
	assert(xsave_area_pgd);
	_context.xsave_area = (uintptr_t)sys.mm().pgalloc().pgd_to_vpa(xsave_area_pgd);

	bzero((void *)_context.xsave_area, 0x1000);*/

	// Prepare the initial stack for this thread.  Threads ALWAYS start in kernel mode, irrespective of whether or
	// not they are user threads.  This stack will set-up the thread context.
	prepare_initial_stack();
}

/**
 * Cleans-up any resources associated with this thread.
 */
Thread::~Thread()
{
	// The VMA will release allocated memory (hopefully)
}

void Thread::add_entry_argument(void* arg)
{
	switch (_current_entry_argument) {
	case 0:
		_context.native_context->rdi = (uint64_t)arg;
		break;
	case 1:
		_context.native_context->rsi = (uint64_t)arg;
		break;
	case 2:
		_context.native_context->rdx = (uint64_t)arg;
		break;
	case 3:
		_context.native_context->rcx = (uint64_t)arg;
		break;

	default:
		return;
	}

	_current_entry_argument++;
}

/**
 * Starts the thread.
 */
void Thread::start()
{
	// Set the state of this thread to be runnable.
	sys.scheduler().set_entity_state(*this, SchedulingEntityState::RUNNABLE);
}

/**
 * Stops the thread.  This does not clean up any resources associated with the thread, however.
 */
void Thread::stop()
{
	// Set the state of this thread to be stopped.
	sys.scheduler().set_entity_state(*this, SchedulingEntityState::STOPPED);

	// If this thread is currently running, then we must yield so that
	// execution doesn't return into it.
	if (&Thread::current() == this) {
		sys.arch().invoke_kernel_syscall(1);
	}
}

void Thread::sleep()
{
	sys.scheduler().set_entity_state(*this, SchedulingEntityState::SLEEPING);

	if (&Thread::current() == this) {
		sys.arch().invoke_kernel_syscall(1);
	}
}

void Thread::wake_up()
{
	sys.scheduler().set_entity_state(*this, SchedulingEntityState::RUNNABLE);
}

/**
 * Activates the thread by making it the one that is currently being run.
 */
bool Thread::activate(SchedulingEntity *prev)
{
	// This thread can only be activated if it is runnable.
	assert(state() == SchedulingEntityState::RUNNABLE);

	// If the previous thread was actually us, then there's nothing to do.
	if (prev == this) return true;

	// Set the current thread to be us.
	sys.arch().set_current_thread(*this);

	// Success!
	return true;
}

void Thread::allocate_user_stack(virt_addr_t vaddr, size_t size)
{
	int nr_pages = __align_up_page(size) >> 12;

	_owner.vma().allocate_virt(vaddr, nr_pages);
	_context.native_context->rsp = vaddr + size - 8;
}

Thread& Thread::current()
{
	return sys.arch().get_current_thread();
}

void Thread::prepare_initial_stack()
{
	uint64_t *stack = (uint64_t *)context().kernel_stack;

	// System Context
	*--stack = is_kernel_thread() ? 0x10 : 0x23;	// SS
	*--stack = 0;									// RSP
	*--stack = 0x202;								// RFLAGS
	*--stack = is_kernel_thread() ? 0x8 : 0x1b;		// CS
	*--stack = (uint64_t)_entry_point;				// RIP
	*--stack = 0;									// EXTRA

	// Thread Context
	*--stack = 0;	// RAX
	*--stack = 0;	// RBX
	*--stack = 0;	// RCX
	*--stack = 0;	// RDX
	*--stack = 0;	// RSI
	*--stack = 0;	// RDI
	*--stack = 0;	// RBP
	*--stack = 0;	// R8
	*--stack = 0;	// R9
	*--stack = 0;	// R10
	*--stack = 0;	// R11
	*--stack = 0;	// R12
	*--stack = 0;	// R13
	*--stack = 0;	// R14
	*--stack = 0;	// R15
	*--stack = 0;	// Previous Context

	_context.native_context = (X86Context *)((uintptr_t)stack);
	_context.native_context->rsp = (uint64_t)context().kernel_stack;
}
