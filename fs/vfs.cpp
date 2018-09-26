/* SPDX-License-Identifier: MIT */

/*
 * fs/vfs.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/fs/vfs.h>
#include <infos/fs/vfs-node.h>
#include <infos/fs/pfs-node.h>
#include <infos/fs/filesystem.h>
#include <infos/fs/file.h>
#include <infos/fs/directory.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/device-manager.h>
#include <infos/util/string.h>
#include <infos/drivers/block/block-device.h>

using namespace infos::drivers;
using namespace infos::drivers::block;
using namespace infos::fs;
using namespace infos::kernel;
using namespace infos::util;

extern char _VFS_REG_START, _VFS_REG_END;

ComponentLog infos::fs::vfs_log(syslog, "vfs");

VirtualFilesystem::VirtualFilesystem(Kernel& owner) : Subsystem(owner)
{
}

bool VirtualFilesystem::init()
{
	vfs_log.message(LogLevel::INFO, "VFS init");
	
	FilesystemRegistration *reg = (FilesystemRegistration *)&_VFS_REG_START;
	
	while (reg < (FilesystemRegistration *)&_VFS_REG_END) {
		vfs_log.messagef(LogLevel::INFO, "Registering filesystem '%s'", reg->name);
		_filesystems.append(reg);
		
		reg++;
	}
	
	Filesystem* rootfs = instantiate_fs("rootfs");
	PFSNode *rootfs_pn = rootfs->mount();
	
	if (!rootfs_pn) {
		return false;
	}
	
	_root_node = new VFSNode(NULL, rootfs_pn);
	if (!_root_node) {
		return false;
	}
	
	return true;
}

File* VirtualFilesystem::open(const String& path, int flags)
{
	VFSNode *node = lookup_node(path);

	if (!node) return NULL;
	if (!node->pn()) return NULL;	
	
	return node->pn()->open();
}

Directory* VirtualFilesystem::opendir(const String& path, int flags)
{
	VFSNode *node = lookup_node(path);
	if (!node) return NULL;
	if (!node->pn()) return NULL;
	
	return node->pn()->opendir();
}

FilesystemRegistration* VirtualFilesystem::lookup_fs(const util::String& fstype) const
{
	FilesystemRegistration *fsreg = NULL;
	for (auto reg : _filesystems) {
		if (strncmp(reg->name, fstype.c_str(), 16) == 0) {
			fsreg = reg;
			break;
		}
	}
	
	return fsreg;
}

Filesystem* VirtualFilesystem::instantiate_fs(const char* fstype, Device *dev)
{
	FilesystemRegistration *fsreg = lookup_fs(fstype);
	if (!fsreg) {
		return NULL;
	}
	
	return fsreg->creation_fn(*this, dev);
}

VFSNode* VirtualFilesystem::lookup_node(const String& spath)
{
	const char *path = spath.c_str();
	fs_log.messagef(LogLevel::DEBUG, "lookup=%s", path);
	
	// Must supply a path.
	if (path[0] == 0) return NULL;
	
	// Must be an absolute path.
	if (path[0] != '/') return NULL;

	// Specialise for the root.
	if (path[1] == 0) return _root_node;
	
	// Start at the root.
	VFSNode *current_node = _root_node;
		
	// Prepare to read path components
	path++;
	
	char component[256];
	do {
		int i = 0;
		while (*path && *path != '/') {
			component[i++] = *path++;
		}
		component[i] = 0;
		if (*path == '/') path++;
		
		if (strlen(component) == 0) {
			return current_node;
		}

		//fs_log.messagef(LogLevel::DEBUG, "component=%s", component);
		current_node = current_node->get_child(component);
		//fs_log.messagef(LogLevel::DEBUG, "node=%p remain=%s", current_node, path);
	} while (current_node);
	
	return current_node;
}
