/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/define.h>

namespace infos
{
	namespace util
	{		
		struct CommandLineArgumentRegistration
		{
			typedef void (*RegistrationFn)(const char *);
			
			const char *match;
			RegistrationFn fn;
		} __packed;
	
		class CommandLine
		{
		public:
			bool parse(const char *cmdline);
			
		private:
			void process(const char *setting);
			const CommandLineArgumentRegistration *find_registration(const char *key);
		};
	
#define RegisterCmdLineArgument(__name, __match) static void __parse##__name(const char *); \
__section(".cmdlineargs") infos::util::CommandLineArgumentRegistration __cmdline_arg##__name = { __match, __parse##__name }; \
__section(".text.clfns") static void __parse##__name(const char *value)
		
#define RegisterSimpleCmdLineArgument(__type, __name, __match) static __type __name; \
RegisterCmdLineArgument(__name, __match) { __name = value; }
		
	}
}
