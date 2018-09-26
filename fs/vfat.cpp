/* SPDX-License-Identifier: MIT */

/*
 * fs/vfat.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/fs/vfat.h>
#include <infos/drivers/block/block-device.h>
#include <infos/kernel/log.h>
#include <infos/util/string.h>

using namespace infos::fs;
using namespace infos::kernel;
using namespace infos::util;
using namespace infos::drivers::block;

VFAT::VFAT(drivers::block::BlockDevice& bdev) : BlockBasedFilesystem(bdev)
{

}

PFSNode *VFAT::mount()
{
	fs_log.messagef(LogLevel::DEBUG, "vfat: block-size=%u, count=%u", block_device().block_size(), block_device().block_size());

	if (block_device().block_size() != sizeof(vfat_boot_block)) {
		return NULL;
	}

	if (!block_device().read_blocks(&_boot_block, 0, 1)) {
		return NULL;
	}
	
	if (_boot_block.signature != 0xaa55) {
		fs_log.messagef(LogLevel::ERROR, "vfat: invalid signature");
		return NULL;
	}
	
	return new VFATNode(*this);
}

VFATNode::VFATNode(VFAT& owner) : PFSNode(NULL, owner)
{

}

PFSNode* VFATNode::get_child(const util::String& name)
{
	return NULL;
}

PFSNode* VFATNode::mkdir(const util::String& name)
{
	return NULL;
}


File* VFATNode::open()
{
	return NULL;
}

Directory* VFATNode::opendir()
{
	return NULL;
}

static Filesystem *vfat_create(VirtualFilesystem& vfs, infos::drivers::Device *dev)
{
	if (!dev->device_class().is(BlockDevice::BlockDeviceClass)) return NULL;
	return new VFAT((BlockDevice &) *dev);
}

RegisterFilesystem(vfat, vfat_create);
