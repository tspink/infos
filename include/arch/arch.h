/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/kernel/irq.h>
#include <infos/kernel/syscall.h>

namespace infos
{
	namespace kernel
	{
		class CPU;
		class IRQ;
		class Thread;
		class ThreadContext;
	}
	
	namespace arch
	{
		class Arch
		{
		public:
			virtual void enable_interrupts() = 0;
			virtual void disable_interrupts() = 0;
			virtual bool interrupts_enabled() = 0;
			
			virtual kernel::CPU& get_current_cpu() = 0;
			
			virtual void dump_current_context() const = 0;
			virtual void dump_thread_context(const kernel::ThreadContext& context) const = 0;
			virtual void dump_stack(const kernel::ThreadContext& context) const = 0;
			
			virtual void invoke_kernel_syscall(int nr) = 0;
			
			virtual kernel::Thread& get_current_thread() const = 0;
			virtual void set_current_thread(kernel::Thread& thread) = 0;
			
			virtual kernel::IRQ *request_irq() = 0;
		};
		
		extern Arch& sys_arch;
	}
}
