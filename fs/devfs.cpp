/* SPDX-License-Identifier: MIT */

/*
 * fs/devfs.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/fs/devfs.h>
#include <infos/fs/vfs.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/log.h>
#include <infos/drivers/device.h>
#include <infos/util/string.h>
#include <infos/drivers/console/virtual-console.h>

using namespace infos::fs;
using namespace infos::util;
using namespace infos::kernel;
using namespace infos::drivers;

PFSNode *DeviceFS::mount()
{
	return new DeviceFSRootNode(*this);
}

DeviceFSRootNode::DeviceFSRootNode(DeviceFS& owner) : PFSNode(NULL, owner)
{

}

PFSNode* DeviceFSRootNode::get_child(const util::String& name)
{
	Device *dev;
	if (!kernel::sys.device_manager().try_get_device_by_name(name, dev)) {
		return NULL;
	}

	return new DeviceFSNode(*this, *dev);
}

PFSNode* DeviceFSRootNode::mkdir(const util::String& name)
{
	return NULL;
}

File* DeviceFSRootNode::open()
{
	return NULL;
}

Directory* DeviceFSRootNode::opendir()
{
	return new DeviceFSDirectory(*this);
}

DeviceFSNode::DeviceFSNode(DeviceFSRootNode& root, drivers::Device& dev) 
	: PFSNode(&root, root.owner()),
	_dev(dev)
{
}

PFSNode* DeviceFSNode::get_child(const util::String& name)
{
	return NULL;
}

PFSNode* DeviceFSNode::mkdir(const util::String& name)
{
	return NULL;
}

File* DeviceFSNode::open()
{
	return _dev.open_as_file();
}

Directory* DeviceFSNode::opendir()
{
	return NULL;
}

DeviceFSDirectory::DeviceFSDirectory(DeviceFSRootNode& node)
{
	for (const auto& device : kernel::sys.device_manager().devices()) {
		DirectoryEntry de;
		de.name = device.value->name();
		de.size = 0;
		
		add_entry(de);
	}
}


static Filesystem *devfs_create(VirtualFilesystem& vfs, infos::drivers::Device *dev)
{
	return new DeviceFS(vfs.owner().device_manager());
}

RegisterFilesystem(devfs, devfs_create);
