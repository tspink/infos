/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/define.h>

namespace infos
{
	namespace kernel
	{
		class DeviceManager;
	}
	
	namespace drivers
	{
		namespace pci
		{
			class PCIDevice;
			class PCIBus
			{
				friend class PCIDevice;
				
			public:
				PCIBus(unsigned int bus_id);
				
				unsigned int id() const { return _bus_id; }
				
				bool probe(kernel::DeviceManager& dm);
				
			private:
				unsigned int _bus_id;
				
				bool probe_slot(kernel::DeviceManager& dm, unsigned int slot);
				bool probe_func(kernel::DeviceManager& dm, unsigned int slot, unsigned int func);
				
				uint32_t read_config(uint8_t slot, uint8_t func, uint8_t reg);
				void write_config(uint8_t slot, uint8_t func, uint8_t reg, uint32_t value);
			};
		}
	}
}
