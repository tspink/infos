/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/fs/filesystem.h>

namespace infos
{
	namespace drivers
	{
		namespace block
		{
			class BlockDevice;
		}
	}
	
	namespace fs
	{
		class BlockBasedFilesystem : public Filesystem
		{
		public:
			typedef BlockBasedFilesystem *(*ProbeFn)(drivers::block::BlockDevice& bdev);
			
			BlockBasedFilesystem(drivers::block::BlockDevice& bdev);
			
		protected:
			drivers::block::BlockDevice& block_device() const { return _bdev; }
			
		private:
			drivers::block::BlockDevice& _bdev;
		};
	}
}
