/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/define.h>
#include <infos/kernel/irq.h>

#define MAX_IRQS 256

namespace infos
{
	namespace arch
	{
		namespace x86
		{
#define IRQ_TRAP			0x03
#define IRQ_PAGE_FAULT		0x0e
#define IRQ_GPF				0x0d
#define IRQ_KERNEL_SYSCALL	0x80
#define IRQ_USER_SYSCALL	0x81
			
			class ExceptionIRQ : public kernel::IRQ
			{
			public:
				void handle() const override;
				void enable() override;
				void disable() override;
			};
					
			class SoftwareIRQ : public kernel::IRQ
			{
			public:
				void handle() const override;
				void enable() override;
				void disable() override;
			};
			
			class IRQDescriptor
			{
			public:
				IRQDescriptor() : _irq(NULL) { }

				kernel::IRQ *irq() const { return _irq; }
				void irq(kernel::IRQ *irq) { _irq = irq; }

			private:
				kernel::IRQ *_irq;
			};
			
			class IRQManager
			{
			public:
				bool init();
				
				bool install_exception_handler(uint8_t nr, kernel::IRQ::irq_handler_t handler, void *priv);
				bool install_software_handler(uint8_t nr, kernel::IRQ::irq_handler_t handler, void *priv);
				
				bool attach_irq(kernel::IRQ *irq);
								
				const IRQDescriptor *get_irq_descriptor(uint8_t nr) const { return &irq_descriptors[nr]; }
				
			private:
				IRQDescriptor irq_descriptors[MAX_IRQS];
				
				template<typename T>
				bool install_handler(uint8_t nr, kernel::IRQ::irq_handler_t handler, void *priv);
			};
			
			static inline uint64_t __save_flags()
			{
				uint64_t flags;
				asm volatile("pushf; pop %0" : "=rm"(flags) :: "memory");
				return flags;
			}

			static inline void __restore_flags(uint64_t flags)
			{
				asm volatile("push %0; popf" :: "r"(flags) : "memory", "cc");
			}
			
			static inline void __irq_enable()
			{
				asm volatile("sti" ::: "memory");
			}

			static inline void __irq_disable()
			{
				asm volatile("sti" ::: "memory");
			}
		}
	}
}
