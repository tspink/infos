/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/fs/exec/loader.h>
#include <infos/kernel/log.h>

namespace infos
{
	namespace fs
	{
		class File;
		
		namespace exec
		{
			class ElfLoader : public Loader
			{
			public:
				ElfLoader(File& f);
				virtual ~ElfLoader() { }
				
				kernel::Process* load(const util::String& cmdline) override;
				
			private:
				File& _file;
			};
			
			extern kernel::ComponentLog elf_log;
		}		
	}
}
