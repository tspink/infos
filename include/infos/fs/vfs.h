/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/kernel/subsystem.h>
#include <infos/kernel/log.h>
#include <infos/fs/vfs-node.h>
#include <infos/util/list.h>
#include <infos/util/string.h>

namespace infos
{
	namespace drivers
	{
		class Device;
	}
	
	namespace fs
	{
		class Filesystem;
		class FilesystemRegistration;
		class File;
		class Directory;
		
		class VFSNode;
		
		class VirtualFilesystem : public kernel::Subsystem
		{
		public:
			VirtualFilesystem(kernel::Kernel& owner);
			
			bool init();
			
			File *open(const util::String& path, int flags);
			Directory *opendir(const util::String& path, int flags);
			
			VFSNode *lookup_node(const util::String& path);
			FilesystemRegistration *lookup_fs(const util::String& fstype) const;
			
		private:
			VFSNode *_root_node;
			
			util::List<FilesystemRegistration *> _filesystems;
			Filesystem *instantiate_fs(const char *fstype, drivers::Device* dev = NULL);		
		};
				
		extern kernel::ComponentLog vfs_log;
	}
}
