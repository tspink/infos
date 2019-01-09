/* SPDX-License-Identifier: MIT */

#pragma once

#include <arch/arch.h>
#include <arch/x86/irq.h>
#include <infos/util/map.h>

extern "C" struct X86Context;

namespace infos
{
	namespace arch
	{
		namespace x86
		{
			class IRQManager;
			
			class X86Arch : public Arch
			{
			public:
				X86Arch();
				
				bool init();
				bool init_irq();
				
				void enable_interrupts() override { asm volatile("sti"); }
				void disable_interrupts() override { asm volatile("cli"); }
				bool interrupts_enabled() override {
					uint64_t rflags;
					asm volatile("pushf ; pop %0" : "=rm"(rflags) : : "memory");
					return !!(rflags & 0x200);
				}

				kernel::CPU& get_current_cpu() override { return *_cpus[0]; }
				
				void dump_native_context(const X86Context& native_context) const;
				void dump_thread_context(const kernel::ThreadContext& context) const override;
				void dump_current_context() const override;
				void dump_stack(const kernel::ThreadContext& context) const override;

				void invoke_kernel_syscall(int nr) override;
				
				kernel::Thread& get_current_thread() const override;
				void set_current_thread(kernel::Thread& thread) override;
				
				kernel::IRQ* request_irq() override;
				
				IRQManager& irq_manager() { return _irq_manager; }
				
			private:
				kernel::CPU *_cpus[1];
				IRQManager _irq_manager;
			};
			
			extern X86Arch x86arch;
		}
	}
}
