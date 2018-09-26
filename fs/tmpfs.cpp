/* SPDX-License-Identifier: MIT */

/*
 * fs/tmpfs.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/fs/tmpfs.h>
#include <infos/util/string.h>
#include <infos/kernel/log.h>

using namespace infos::fs;
using namespace infos::util;
using namespace infos::kernel;

PFSNode *TempFS::mount()
{
	return new TempFSNode(*this, "");
}

TempFSNode::TempFSNode(TempFS& owner, const util::String& name) : PFSNode(NULL, owner), _name(name)
{

}

PFSNode* TempFSNode::get_child(const util::String& name)
{
	TempFSNode *child;
	
	if (!_children.try_get_value(name.get_hash(), child)) {
		return NULL;
	}

	return child;
}

PFSNode* TempFSNode::mkdir(const util::String& name)
{
	TempFSNode *dir = new TempFSNode((TempFS &)owner(), name);
	_children.add(name.get_hash(), dir);
	
	return dir;
}

File* TempFSNode::open()
{
	return NULL;
}

Directory* TempFSNode::opendir()
{
	return new TempFSDirectory(*this);
}

TempFSDirectory::TempFSDirectory(TempFSNode& node)
{
	for (const auto& child : node.children()) {
		DirectoryEntry de;
		de.name = child.value->name();
		de.size = 0;
		
		add_entry(de);
	}
}

static Filesystem *tmpfs_create(VirtualFilesystem& vfs, infos::drivers::Device *dev)
{
	return new TempFS();
}

RegisterFilesystem(tmpfs, tmpfs_create);
