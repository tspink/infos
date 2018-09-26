/* SPDX-License-Identifier: MIT */

/*
 * drivers/block/block-device-partition.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2018.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/block/block-device-partition.h>

using namespace infos::drivers;
using namespace infos::drivers::block;

const DeviceClass BlockDevicePartition::BlockDevicePartitionClass(BlockDevice::BlockDeviceClass, "block-partition");

BlockDevicePartition::BlockDevicePartition(BlockDevice& underlying_block_device, size_t block_offset, size_t block_count) : _underlying_block_device(underlying_block_device), _block_offset(block_offset), _block_count(block_count)
{

}

BlockDevicePartition::~BlockDevicePartition()
{

}

bool BlockDevicePartition::read_blocks(void* buffer, size_t offset, size_t count)
{
	return _underlying_block_device.read_blocks(buffer, _block_offset + offset, count);
}

bool BlockDevicePartition::write_blocks(const void* buffer, size_t offset, size_t count)
{
	return _underlying_block_device.write_blocks(buffer, _block_offset + offset, count);
}
