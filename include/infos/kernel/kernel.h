/* SPDX-License-Identifier: MIT */

/*
 * include/kernel/kernel.h
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/kernel/om.h>
#include <infos/kernel/device-manager.h>
#include <infos/kernel/module.h>
#include <infos/kernel/sched.h>
#include <infos/kernel/syscall.h>
#include <infos/mm/mm.h>
#include <infos/fs/vfs.h>
#include <infos/util/time.h>
#include <infos/util/cmdline.h>

namespace infos
{
	namespace arch
	{
		class Arch;
	}

	namespace kernel
	{
		class Process;

		class Kernel
		{
		public:
			typedef bool (*BottomFn)(void);

			Kernel(arch::Arch& arch);

			bool early_init(const char *cmdline);

			__noreturn void start(BottomFn bottom);

			inline arch::Arch& arch() const { return _arch; }

			inline ObjectManager& object_manager() { return _object_manager; }
			inline DeviceManager& device_manager() { return _device_manager; }
			inline mm::MemoryManager& mm() { return _memory_manager; }
			inline ModuleManager& module_manager() { return _module_manager; }
			inline Scheduler& scheduler() { return _scheduler; }
			inline fs::VirtualFilesystem& vfs() { return _vfs; }
			inline util::CommandLine& cmdline() { return _cmdline; }
			inline SyscallManager& syscalls() { return _scm; }

			void update_runtime(util::Nanoseconds ns);
			void print_tod();

			const util::KernelRuntimeClock::Timepoint runtime() const { asm volatile("" ::: "memory"); return _runtime; }

			inline void spin_delay(util::Seconds s) { spin_delay(util::DurationCast<util::Nanoseconds>(s)); }
			inline void spin_delay(util::Milliseconds s) { spin_delay(util::DurationCast<util::Nanoseconds>(s)); }
			inline void spin_delay(util::Microseconds s) { spin_delay(util::DurationCast<util::Nanoseconds>(s)); }
			void spin_delay(util::Nanoseconds ns);

			Process *launch_process(const util::String& path, const util::String& cmdline);

			const util::TimeOfDay& time_of_day() const { return _tod; }

		private:
			arch::Arch& _arch;
			ObjectManager _object_manager;
			DeviceManager _device_manager;
			mm::MemoryManager _memory_manager;
			ModuleManager _module_manager;
			Scheduler _scheduler;
			fs::VirtualFilesystem _vfs;
			util::CommandLine _cmdline;
			SyscallManager _scm;

			util::KernelRuntimeClock::Timepoint _runtime;
			util::TimeOfDay _tod;

			util::Nanoseconds _ticks_since_last_tod_update;

			Process *_kernel_process;

			static void start_kernel_threadproc_tramp(Kernel *kernel, BottomFn bottom);
			void start_kernel_threadproc();
            void dump_partitions();
			void initialise_tod();
			void resync_tod();
			void increment_tod();
		};

		extern Kernel sys;
	}
}
