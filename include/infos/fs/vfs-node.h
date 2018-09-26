/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/fs/fs-node.h>
#include <infos/util/map.h>

namespace infos
{
	namespace drivers
	{
		class Device;
	}
	
	namespace fs
	{
		class Filesystem;
		class PFSNode;
		
		class VFSNode : public FSNode<VFSNode>
		{
		public:
			VFSNode(VFSNode *parent, PFSNode *pn = NULL) : FSNode(parent), _pn(pn) { }

			VFSNode* get_child(const util::String& name) override;
			VFSNode* mkdir(const util::String& name) override;

			PFSNode* pn() const { return _pn; }
						
			Filesystem *mount(const util::String& fstype, drivers::Device *dev);
			
		private:
			PFSNode *_pn;
			util::Map<util::String::hash_type, VFSNode *> _children;
		};
	}
}
