/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/drivers/device.h>
#include <infos/kernel/irq.h>

namespace infos
{
	namespace drivers
	{
		namespace irq
		{
			class LAPIC;
			
			class IOAPIC : public Device
			{
			public:
				static const DeviceClass IOAPICDeviceClass;
				const DeviceClass& device_class() const override { return IOAPICDeviceClass; }
				
				IOAPIC(virt_addr_t base_address);
				
				bool init(kernel::DeviceManager& dm) override;
				
				kernel::IRQ *request_physical_irq(LAPIC *lapic, uint32_t phys_irq_nr);
				
			private:
				class IOAPICIRQ : public kernel::IRQ
				{
				public:
					IOAPICIRQ(LAPIC& lapic) : _lapic(lapic) { }
					
					void handle() const override;
					void enable() override;
					void disable() override;
					
				private:
					LAPIC& _lapic;
				};
				
				volatile uint32_t *_base_address;
				uint8_t _nr_irqs;
				
				void write(uint32_t reg, uint32_t val)
				{
					_base_address[0] = reg;
					__sync_synchronize();
					_base_address[4] = val;
				}

				uint32_t read (uint32_t reg)
				{
					_base_address[0] = reg;
					__sync_synchronize();
					return _base_address[4];
				}
				
				struct RedirectionEntry
				{
					union {
						struct { uint64_t low, high; } __packed;

						struct {
							uint8_t vector;
							uint8_t delivery_mode : 3;
							uint8_t destination_mode : 1;
							uint8_t delivery_status : 1;
							uint8_t pin_polarity : 1;
							uint8_t remote_irr : 1;
							uint8_t trigger_mode : 1;
							uint8_t mask : 1;
							uint8_t destination;
						} __packed;
					};
				} __packed;
				
				void store_redir_entry(uint8_t irq_index, RedirectionEntry& e);
			};
		}
	}
}
