/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/util/string.h>

namespace infos
{
	namespace kernel
	{
		class Process;
	}
	
	namespace fs
	{
		namespace exec
		{
			class Loader
			{
			public:
				virtual ~Loader() { }
				virtual kernel::Process *load(const util::String& cmdline) = 0;
			};
		}
	}
}
