/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/define.h>

namespace infos
{
	namespace fs
	{
		class File
		{
		public:
			enum SeekType
			{
				SeekAbsolute,
				SeekRelative,
			};

			virtual ~File() { }

			virtual int read(void *buffer, size_t size) { return 0; }
			virtual int pread(void *buffer, size_t size, off_t off) { return 0; }
			virtual int write(const void *buffer, size_t size) { return 0; }
			virtual int pwrite(const void *buffer, size_t size, off_t off) { return 0; }
			virtual void seek(off_t offset, SeekType type) { }

			virtual void close() { }
		};
	}
}
