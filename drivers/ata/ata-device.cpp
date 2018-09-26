/* SPDX-License-Identifier: MIT */

/*
 * drivers/ata/ata-device.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/ata/ata-device.h>
#include <infos/drivers/ata/ata-controller.h>
#include <infos/kernel/kernel.h>
#include <infos/util/lock.h>
#include <infos/util/string.h>
#include <arch/x86/pio.h>

using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::drivers::ata;
using namespace infos::drivers::block;
using namespace infos::util;
using namespace infos::arch::x86;

const DeviceClass ATADevice::ATADeviceClass(BlockDevice::BlockDeviceClass, "ata");

ATADevice::ATADevice(ATAController& controller, int channel, int drive)
: _ctrl(controller),
_channel(channel & 1),
_drive(drive & 1)
{

}

bool ATADevice::init(kernel::DeviceManager& dm)
{
	uint8_t *buffer = new uint8_t[128 * 4];
	if (!buffer)
		return false;

	ata_read_buffer(ATA_REG_DATA, buffer, 128 * 4);

	_signature = *(uint16_t *) (buffer + ATA_IDENT_DEVICETYPE);
	_caps = *(uint16_t *) (buffer + ATA_IDENT_CAPABILITIES);
	_cmdsets = *(uint32_t *) (buffer + ATA_IDENT_COMMANDSETS);

	if (_cmdsets & (1 << 26)) {
		_size = *(uint32_t *) (buffer + ATA_IDENT_MAX_LBA_EXT);
	} else {
		_size = *(uint32_t *) (buffer + ATA_IDENT_MAX_LBA);
	}

	char model[41];
	for (int i = 0; i < 40; i += 2) {
		model[i] = buffer[ATA_IDENT_MODEL + i + 1];
		model[i + 1] = buffer[ATA_IDENT_MODEL + i];
	}

	for (int i = 40; i > 0; i--) {
		model[i] = 0;
		if (model[i - 1] != ' ') break;
	}

	ata_log.messagef(LogLevel::DEBUG, "model=%s, size=%u, caps=%x", model, _size, _caps);

	sys.mm().objalloc().free(buffer);

	if ((_caps & 0x200) == 0) {
		ata_log.messagef(LogLevel::ERROR, "drive does not support lba addressing mode");
		return false;
	}
	
	return check_for_partitions();
}

bool ATADevice::check_for_partitions()
{
	uint8_t *buffer = new uint8_t[512];
	if (!buffer)
		return false;
	
	if (!read_blocks(buffer, 0, 1)) {
		sys.mm().objalloc().free(buffer);
		return false;
	}
	
	bool result = true;
	if (buffer[0x1fe] == 0x55 && buffer[0x1ff] == 0xaa) {
		ata_log.messagef(LogLevel::INFO, "disk has partitions!");
		result = create_partitions(buffer);
	}
	
	sys.mm().objalloc().free(buffer);
	return result;
}

bool ATADevice::create_partitions(const uint8_t* partition_table)
{
	struct partition_table_entry {
		uint8_t status;
		uint8_t first_absolute_sector[3];
		uint8_t type;
		uint8_t last_absolute_sector[3];
		uint32_t first_absolute_sector_lba;
		uint32_t nr_sectors;
	} __packed;
		
	for (int partition_table_index = 0; partition_table_index < 4; partition_table_index++) {
		const struct partition_table_entry *pte = (const partition_table_entry *)&partition_table[0x1be + (16 * partition_table_index)];
		
		if (pte->type == 0) {
			ata_log.messagef(LogLevel::INFO, "partition %u inactive", partition_table_index);
			continue;
		}
		
		ata_log.messagef(LogLevel::INFO, "partition %u active @ off=%x, sz=%x", partition_table_index, pte->first_absolute_sector, pte->nr_sectors);
		
		auto partition_device = new infos::drivers::block::BlockDevicePartition(*this, pte->first_absolute_sector_lba, pte->nr_sectors);
		_partitions.append(partition_device);

		sys.device_manager().register_device(*partition_device);

		String partition_name = name() + "p" + ToString(partition_table_index);
		sys.device_manager().add_device_alias(partition_name, *partition_device);
	}
	
	return true;
}

size_t ATADevice::block_count() const
{
	return _size;
}

size_t ATADevice::block_size() const
{
	return 512;
}

bool ATADevice::read_blocks(void* buffer, size_t offset, size_t count)
{
	return transfer(0, offset, buffer, count);
}

bool ATADevice::write_blocks(const void* buffer, size_t offset, size_t count)
{
	return transfer(1, offset, (void *) buffer, count);
}

uint8_t ATADevice::ata_read(int reg)
{
	return _ctrl.ata_read(_channel, reg);
}

void ATADevice::ata_read_buffer(int reg, void* buffer, size_t size)
{
	return _ctrl.ata_read_buffer(_channel, reg, buffer, size);
}

void ATADevice::ata_write(int reg, uint8_t data)
{
	_ctrl.ata_write(_channel, reg, data);
}

int ATADevice::ata_poll(bool error_check)
{
	return _ctrl.ata_poll(_channel, error_check);
}

bool ATADevice::transfer(int direction, uint64_t lba, void* buffer, size_t nr_blocks)
{
	UniqueLock<Mutex> l(_ctrl._mtx[_channel]);

	while (ata_read(ATA_REG_STATUS) & ATA_SR_BSY) asm volatile("pause");

	ata_write(ATA_REG_HDDEVSEL, 0xE0 | (_drive << 4));

	ata_write(ATA_REG_SECCOUNT1, 0);
	ata_write(ATA_REG_LBA3, (lba >> 24) & 0xff);
	ata_write(ATA_REG_LBA4, (lba >> 32) & 0xff);
	ata_write(ATA_REG_LBA5, (lba >> 40) & 0xff);

	ata_write(ATA_REG_SECCOUNT0, nr_blocks);
	ata_write(ATA_REG_LBA0, (lba >> 0) & 0xff);
	ata_write(ATA_REG_LBA1, (lba >> 8) & 0xff);
	ata_write(ATA_REG_LBA2, (lba >> 16) & 0xff);

	if (direction) {
		ata_write(ATA_REG_COMMAND, ATA_CMD_WRITE_PIO_EXT);
	} else {
		ata_write(ATA_REG_COMMAND, ATA_CMD_READ_PIO_EXT);

		uint8_t *p = (uint8_t *) buffer;
		for (size_t cur_block = 0; cur_block < nr_blocks; cur_block++) {
			if (ata_poll(true)) {
				return false;
			}
			
			__insw(_ctrl.channels[_channel].base, (uintptr_t)p, (512 / 2));

			p += 512;
		}
	}

	return true;
}
