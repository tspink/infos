/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/fs/fs-node.h>

namespace infos
{
	namespace fs
	{
		class Filesystem;
		class File;
		class Directory;
		
		class PFSNode : public FSNode<PFSNode>
		{
		public:
			PFSNode(PFSNode *parent, Filesystem& owner) : FSNode(parent), _owner(owner) { }
			
			virtual File *open() = 0;
			virtual Directory *opendir() = 0;
			
			Filesystem& owner() const { return _owner; }
			
		private:
			Filesystem& _owner;
		};
	}
}
