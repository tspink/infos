/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/fs/filesystem.h>
#include <infos/fs/pfs-node.h>

namespace infos
{
	namespace fs
	{
		class RootFS : public Filesystem
		{
		public:
			PFSNode *mount() override;
			
			const util::String name() const { return "rootfs"; }
		};

		class RootFSNode : public PFSNode
		{
		public:
			RootFSNode(RootFS& fs);

			PFSNode* get_child(const util::String& name) override;
			PFSNode* mkdir(const util::String& name) override;

			File* open() override;
			Directory* opendir() override;
		};
	}
}
