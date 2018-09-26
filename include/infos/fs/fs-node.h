/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/util/string.h>

namespace infos
{
	namespace fs
	{
		template<class ChildFSNode>
		class FSNode
		{
		public:
			FSNode(ChildFSNode *parent) : _parent(parent) { }
			virtual ~FSNode() { }
			
			virtual ChildFSNode *get_child(const util::String& name) = 0;
			ChildFSNode *parent() const { return _parent; }
			
			virtual ChildFSNode *mkdir(const util::String& name) = 0;

		private:
			ChildFSNode *_parent;
		};
	}
}
