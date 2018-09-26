/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/define.h>
#include <infos/kernel/log.h>
#include <infos/util/string.h>

namespace infos
{
	namespace drivers
	{
		class Device;
	}
	
	namespace fs
	{
		class VirtualFilesystem;
		class PFSNode;
		
		class Filesystem
		{
		public:
			virtual ~Filesystem() { }
			
			typedef Filesystem *(*CreationFn)(VirtualFilesystem&, drivers::Device *);
			
			virtual const util::String name() const = 0;
			
			virtual PFSNode *mount() = 0;
		};
		
		extern kernel::ComponentLog fs_log;
		
		struct FilesystemRegistration
		{
			const char *name;
			Filesystem::CreationFn creation_fn;
		};
	}
}

#define RegisterFilesystem(__name, __creation_fn) __section(".fsreg") infos::fs::FilesystemRegistration __fs_reg##__name = { \
.name = STRINGIFY(__name), \
.creation_fn = __creation_fn \
}
