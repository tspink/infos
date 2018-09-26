/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/drivers/block/block-device.h>

namespace infos {
    namespace drivers {
        namespace block {

            class BlockDevicePartition : public BlockDevice {
            public:
                static const DeviceClass BlockDevicePartitionClass;

                const DeviceClass& device_class() const override {
                    return BlockDevicePartitionClass;
                }
                
                BlockDevicePartition(BlockDevice& underlying_block_device, size_t block_offset, size_t block_count);
                virtual ~BlockDevicePartition();

                virtual bool read_blocks(void *buffer, size_t offset, size_t count);
                virtual bool write_blocks(const void *buffer, size_t offset, size_t count);

                virtual size_t block_size() const { return _underlying_block_device.block_size(); }
                virtual size_t block_count() const { return _block_count; }
                
            private:
                BlockDevice& _underlying_block_device;
                size_t _block_offset, _block_count;
            };
        }
    }
}
