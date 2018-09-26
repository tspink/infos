/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/drivers/device.h>

namespace infos
{
	namespace drivers
	{
		namespace block
		{
			class BlockDevice : public Device
			{
			public:
				static const DeviceClass BlockDeviceClass;
				const DeviceClass& device_class() const override { return BlockDeviceClass; }
				
				virtual bool read_blocks(void *buffer, size_t offset, size_t count) = 0;
				virtual bool write_blocks(const void *buffer, size_t offset, size_t count) = 0;
				
				virtual size_t block_size() const = 0;
				virtual size_t block_count() const = 0;
			};
		}
	}
}
