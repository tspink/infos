/* SPDX-License-Identifier: MIT */

/*
 * kernel/process.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/process.h>

using namespace infos::kernel;

Process::Process(const util::String& name, bool kernel_process, Thread::thread_proc_t entry_point)
	: _name(name), _kernel_process(kernel_process), _terminated(false), _vma()
{
	// Initialise the VMA by installing the default kernel mapping.
	_vma.install_default_kernel_mapping();

	// Create the main thread.
	_main_thread = &create_thread(kernel_process ? ThreadPrivilege::Kernel : ThreadPrivilege::User, entry_point, "main");
}

Process::~Process()
{
	// All threads /should/ be stopped by this point.
	for (const auto& thread : _threads) {
		assert(thread->state() == SchedulingEntityState::STOPPED);
		delete thread;
	}
}

void Process::start()
{
	// Start the main thread.
	main_thread().start();
}

void Process::terminate(int rc)
{
	_terminated = true;
	_state_changed.trigger();

	for (const auto& thread : _threads) {
		thread->stop();
	}
}

Thread& Process::create_thread(ThreadPrivilege::ThreadPrivilege privilege, Thread::thread_proc_t entry_point, const util::String& name)
{
	// A kernel process can NEVER have a user thread.
	assert(!(kernel_process() && privilege == ThreadPrivilege::User));

	Thread *new_thread = new Thread(*this, privilege, entry_point, name);
	_threads.append(new_thread);

	return *new_thread;
}
