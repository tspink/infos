/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/define.h>
#include <infos/util/string.h>
#include <infos/util/list.h>

namespace infos
{
	namespace fs
	{
		struct DirectoryEntry
		{
			util::String name;
			unsigned int size;
		};
		
		class Directory
		{
		public:
			virtual ~Directory() { }
		
			virtual bool read_entry(DirectoryEntry& entry) = 0;
			
			virtual void close() = 0;
		};
		
		class SimpleDirectory : public Directory
		{
		public:
			SimpleDirectory() : _current_entry(0) { }
			
			bool read_entry(DirectoryEntry& entry) override;
			void close() override;
			
		protected:
			void add_entry(const DirectoryEntry& e);
			
		private:
			util::List<DirectoryEntry> _entries;
			unsigned int _current_entry;
		};
	}
}
