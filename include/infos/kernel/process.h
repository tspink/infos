/* SPDX-License-Identifier: MIT */

/*
 * include/kernel/process.h
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/kernel/thread.h>
#include <infos/mm/vma.h>
#include <infos/util/list.h>
#include <infos/util/string.h>
#include <infos/util/event.h>

namespace infos
{
	namespace kernel
	{
		class Process
		{
		public:
			Process(const util::String& name, bool kernel_process, Thread::thread_proc_t entry_point);
			virtual ~Process();

			const util::String& name() const { return _name; }

			void start();
			void terminate(int rc);
			bool terminated() const { return _terminated; }

			inline bool kernel_process() const { return _kernel_process; }

			mm::VMA& vma() { return _vma; }
			Thread& main_thread() const { return *_main_thread; }

			Thread& create_thread(ThreadPrivilege::ThreadPrivilege privilege, Thread::thread_proc_t entry_point, const util::String& name);

			util::Event& state_changed() { return _state_changed; }

		private:
			const util::String _name;
			bool _kernel_process, _terminated;
			mm::VMA _vma;
			util::List<Thread *> _threads;
			Thread *_main_thread;

			util::Event _state_changed;
		};
	}
}
