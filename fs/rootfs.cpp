/* SPDX-License-Identifier: MIT */

/*
 * fs/rootfs.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/fs/rootfs.h>
#include <infos/util/string.h>

using namespace infos::fs;
using namespace infos::util;

PFSNode *RootFS::mount()
{
	return new RootFSNode(*this);
}

RootFSNode::RootFSNode(RootFS& owner) : PFSNode(NULL, owner)
{

}

PFSNode* RootFSNode::get_child(const util::String& name)
{
	return NULL;
}

PFSNode* RootFSNode::mkdir(const util::String& name)
{
	return NULL;
}

File* RootFSNode::open()
{
	return NULL;
}

Directory* RootFSNode::opendir()
{
	return NULL;
}

/**
 * Constructor to create a rootfs filesystem.
 * @param vfs The VFS instance to which this instance belongs.
 * @param dev Any associated device.
 * @return 
 */
static Filesystem *rootfs_create(VirtualFilesystem& vfs, infos::drivers::Device *dev)
{
	return new RootFS();
}

RegisterFilesystem(rootfs, rootfs_create);
