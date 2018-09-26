/* SPDX-License-Identifier: MIT */
#pragma once


#include <infos/drivers/block/block-device.h>
#include <infos/drivers/block/block-device-partition.h>
#include <infos/util/list.h>

namespace infos {
    namespace drivers {
        namespace ata {
            class ATAController;

            class ATADevice : public block::BlockDevice {
            public:
                static const DeviceClass ATADeviceClass;

                const DeviceClass& device_class() const override {
                    return ATADeviceClass;
                }

                ATADevice(ATAController& controller, int channel, int drive);

                bool init(kernel::DeviceManager& dm) override;

                size_t block_count() const override;
                size_t block_size() const override;
                bool read_blocks(void* buffer, size_t offset, size_t count) override;
                bool write_blocks(const void* buffer, size_t offset, size_t count) override;

            private:
                ATAController& _ctrl;
                int _channel, _drive;

                uint32_t _signature, _caps, _cmdsets, _size;

                uint8_t ata_read(int reg);
                void ata_read_buffer(int reg, void *buffer, size_t size);
                void ata_write(int reg, uint8_t data);
                int ata_poll(bool error_check = false);

                bool transfer(int direction, uint64_t lba, void *buffer, size_t nr_blocks);

                bool check_for_partitions();
                bool create_partitions(const uint8_t *partition_table);
                
                infos::util::List<block::BlockDevicePartition *> _partitions;
            };
        }
    }
}
