/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/fs/filesystem.h>
#include <infos/fs/directory.h>
#include <infos/fs/pfs-node.h>
#include <infos/util/map.h>

namespace infos
{
	namespace fs
	{
		class TempFS : public Filesystem
		{
		public:
			PFSNode *mount() override;
			
			const util::String name() const { return "tmpfs"; }
		};
		
		class TempFSNode : public PFSNode
		{
		public:
			TempFSNode(TempFS& fs, const util::String& name);

			PFSNode* get_child(const util::String& name) override;
			PFSNode* mkdir(const util::String& name) override;
			
			File* open() override;
			Directory* opendir() override;
			
			const util::Map<util::String::hash_type, TempFSNode *>& children() const { return _children; }
			
			const util::String& name() const { return _name; }
			
		private:
			const util::String _name;
			util::Map<util::String::hash_type, TempFSNode *> _children;
		};
		
		class TempFSDirectory : public SimpleDirectory
		{
		public:
			TempFSDirectory(TempFSNode& node);
		};
	}
}
