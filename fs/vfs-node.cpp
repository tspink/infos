/* SPDX-License-Identifier: MIT */

/*
 * fs/vfs-node.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/fs/vfs-node.h>
#include <infos/fs/pfs-node.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/log.h>
#include <infos/fs/vfs.h>
#include <infos/fs/filesystem.h>

using namespace infos::fs;
using namespace infos::kernel;

Filesystem *VFSNode::mount(const util::String& fstype, drivers::Device* dev)
{
	FilesystemRegistration *fsreg = sys.vfs().lookup_fs(fstype);
	if (!fsreg) {
		return NULL;
	}
	
	Filesystem *fs = fsreg->creation_fn(sys.vfs(), dev);
	if (!fs) {
		return NULL;
	}

	vfs_log.messagef(LogLevel::INFO, "vfsnode: mounting filesystem '%s'", fsreg->name);
	PFSNode *pn = fs->mount();
	
	if (!pn) {
		delete fs;
		return NULL;
	}
	
	_pn = pn;
	_children.clear();
	
	//vfs_log.messagef(LogLevel::DEBUG, "vfsnode: mount vfs=%p pfs=%p", this, pn);
	return fs;
}

VFSNode* VFSNode::get_child(const util::String& name)
{
	if (!_pn) return NULL;
	
	VFSNode *child;
	if (!_children.try_get_value(name.get_hash(), child)) {
		PFSNode *assoc = _pn->get_child(name);
		if (!assoc) {
			return NULL;
		}
		
		child = new VFSNode(this, assoc);
		_children.add(name.get_hash(), child);
	}
	
	//vfs_log.messagef(LogLevel::DEBUG, "vfsnode: get child %s vfs=%p pfs=%p child-vfs=%p", name.c_str(), this, _pn, child);
	return child;
}

VFSNode* VFSNode::mkdir(const util::String& name)
{
	if (!_pn) return NULL;
	
	PFSNode *dir = _pn->mkdir(name);
	if (!dir) return NULL;
	
	return get_child(name);
}
