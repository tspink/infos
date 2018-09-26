/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/fs/filesystem.h>
#include <infos/fs/file.h>
#include <infos/fs/directory.h>
#include <infos/fs/pfs-node.h>

namespace infos
{
	namespace drivers
	{
		class Device;
	}
	
	namespace kernel
	{
		class DeviceManager;
	}
	
	namespace fs
	{
		class DeviceFS : public Filesystem
		{
		public:
			DeviceFS(kernel::DeviceManager& dm) : _dm(dm) { }
			
			const util::String name() const { return "devfs"; }
			
			PFSNode *mount() override;
			
		private:
			kernel::DeviceManager& _dm;
		};
		
		class DeviceFSRootNode : public PFSNode
		{
		public:
			DeviceFSRootNode(DeviceFS& fs);
			
			PFSNode* get_child(const util::String& name) override;
			PFSNode* mkdir(const util::String& name) override;
			
			File* open() override;
			Directory* opendir() override;
		};

		class DeviceFSNode : public PFSNode
		{
		public:
			DeviceFSNode(DeviceFSRootNode& fs, drivers::Device& dev);

			PFSNode* get_child(const util::String& name) override;
			PFSNode* mkdir(const util::String& name) override;

			File* open() override;
			Directory* opendir() override;
			
		private:
			drivers::Device& _dev;
		};
		
		class DeviceFSDirectory : public SimpleDirectory
		{
		public:
			DeviceFSDirectory(DeviceFSRootNode& node);
		};
	}
}
