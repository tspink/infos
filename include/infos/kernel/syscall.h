/* SPDX-License-Identifier: MIT */

#pragma once
#include <infos/kernel/om.h>

namespace infos
{
	namespace kernel
	{
		namespace Syscalls
		{
			enum Syscalls
			{
				SYS_NOP		= 0,
				SYS_YIELD	= 1,
				SYS_EXIT	= 2,

				SYS_OPEN	= 3,
				SYS_CLOSE	= 4,
				SYS_READ	= 5,
				SYS_WRITE	= 6,

				SYS_OPENDIR	= 7,
				SYS_READDIR = 8,
				SYS_CLOSEDIR = 9,

				SYS_EXEC	= 10,
				SYS_WAIT_PROC	= 11,
				SYS_CREATE_THREAD = 12,
				SYS_STOP_THREAD = 13,
				SYS_JOIN_THREAD = 14,
				SYS_USLEEP = 15
			};
		}
		
		class Syscall
		{
		public:
			static ObjectHandle sys_open(uintptr_t filename, uint32_t flags);
			static unsigned int sys_close(ObjectHandle h);
			
			static unsigned int sys_read(ObjectHandle h, uintptr_t buffer, size_t size);
			static unsigned int sys_write(ObjectHandle h, uintptr_t buffer, size_t size);
			
			static ObjectHandle sys_opendir(uintptr_t path, uint32_t flags);
			static unsigned int sys_readdir(ObjectHandle h, uintptr_t buffer);
			static unsigned int sys_closedir(ObjectHandle h);
			
			static void sys_exit(unsigned int rc);
			
			static ObjectHandle sys_exec(uintptr_t program, uintptr_t args);
			static unsigned int sys_wait_proc(ObjectHandle h);
			
			static ObjectHandle sys_create_thread(uintptr_t entry_point, uintptr_t arg);
			static unsigned int sys_stop_thread(ObjectHandle h);
			static unsigned int sys_join_thread(ObjectHandle h);
			static unsigned long sys_usleep(unsigned long us);
		};
	}
}
