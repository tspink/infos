/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/drivers/device.h>
#include <infos/kernel/irq.h>

namespace infos
{
	namespace drivers
	{
		namespace timer
		{
			class LAPICTimer;
		}

		namespace irq
		{
			namespace LAPICRegisters {
				enum LAPICRegisters {
					// Local APIC registers, divided by 4 for use as uint[] indices.
					ID = 0x0020, // ID
					VER = 0x0030, // Version
					TPR = 0x0080, // Task Priority
					EOI = 0x00B0, // EOI
					SVR = 0x00F0, // Spurious Interrupt Vector
					//ENABLE     =0x00000100,   // Unit Enable
					ESR = 0x0280, // Error Status
					ICRLO = 0x0300, // Interrupt Command
					//INIT       =0x00000500,   // INIT/RESET
					//STARTUP    =0x00000600,   // Startup IPI
					//DELIVS     =0x00001000,   // Delivery status
					//ASSERT     =0x00004000,   // Assert interrupt =vs deassert
					//DEASSERT   =0x00000000,
					//LEVEL      =0x00008000,   // Level triggered
					//BCAST      =0x00080000,   // Send to all APICs, including self.
					//BUSY       =0x00001000,
					//FIXED      =0x00000000,
					ICRHI = 0x0310, // Interrupt Command [63:32]
					TIMER = 0x0320, // Local Vector Table 0 =TIMER
					//X1         =0x0000000B,   // divide counts by 1
					//PERIODIC   =0x00020000,   // Periodic
					PCINT = 0x0340, // Performance Counter LVT
					LINT0 = 0x0350, // Local Vector Table 1 =LINT0
					LINT1 = 0x0360, // Local Vector Table 2 =LINT1
					ERROR = 0x0370, // Local Vector Table 3 =ERROR
					//MASKED     =0x00010000,   // Interrupt masked
					TICR = 0x0380, // Timer Initial Count
					TCCR = 0x0390, // Timer Current Count
					TDCR = 0x03E0, // Timer Divide Configuration
				};
			}

			class LAPIC : public Device
			{
				friend class timer::LAPICTimer;

			public:
				enum LVTs
				{
					Timer,
					PMU,
					LINT0,
					LINT1,
					Error,
				};

				static const DeviceClass LAPICDeviceClass;
				const DeviceClass& device_class() const override { return LAPICDeviceClass; }

				LAPIC(virt_addr_t base_address);

				bool init(kernel::DeviceManager& dm) override;

				void mask_interrupts(LVTs lvt);
				void unmask_interrupts(LVTs lvt);

				void eoi();

				void set_timer_divide(uint8_t v);
				void set_timer_initial_count(uint32_t v);
				void set_timer_periodic();
				void set_timer_one_shot();
				uint32_t get_timer_current_count();

				kernel::IRQ& timer_irq() const { return *_timer_irq; }

			private:
				class LAPICIRQ : public kernel::IRQ
				{
				public:
					LAPICIRQ(LAPIC& lapic, LVTs lvt) : _lapic(lapic), _lvt(lvt) { }

					void enable() override;
					void disable() override;
					void handle() const override;

				private:
					LAPIC& _lapic;
					LVTs _lvt;
				};

				LAPICIRQ *_timer_irq;

				void set_timer_irq(uint8_t irq);

				volatile uint32_t *_apic_base;
				inline void write(LAPICRegisters::LAPICRegisters reg, uint32_t value) {
					_apic_base[reg >> 2] = value;
					_apic_base[LAPICRegisters::ID >> 2];
				}

				inline uint32_t read(LAPICRegisters::LAPICRegisters reg) const
				{
					__sync_synchronize();
					return _apic_base[reg >> 2];
				}

				inline void set_flag(LAPICRegisters::LAPICRegisters reg, uint32_t value)
				{
					uint32_t oldval = read(reg);
					write(reg, oldval | value);
				}

				inline void clear_flag(LAPICRegisters::LAPICRegisters reg, uint32_t value)
				{
					uint32_t oldval = read(reg);
					write(reg, oldval & (~value));
				}
			};
		}
	}
}
