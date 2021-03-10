/* SPDX-License-Identifier: MIT */

#pragma once
#include <infos/kernel/om.h>

namespace infos {
    namespace kernel {

        class SyscallManager {
        public:
            static const int MAX_SYSCALLS = 256;

            typedef unsigned long (*syscallfn)(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5);

            SyscallManager();

            void RegisterSyscall(int nr, syscallfn fn);
            unsigned long InvokeSyscall(int nr, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5);

        private:
            syscallfn syscall_table_[MAX_SYSCALLS];
        };

        class DefaultSyscalls {
        public:
            static void sys_nop();
            static void sys_yield();

            static ObjectHandle sys_open(uintptr_t filename, uint32_t flags);
            static unsigned int sys_close(ObjectHandle h);

            static unsigned int sys_read(ObjectHandle h, uintptr_t buffer, size_t size);
            static unsigned int sys_write(ObjectHandle h, uintptr_t buffer, size_t size);
            static unsigned int sys_pread(ObjectHandle h, uintptr_t buffer, size_t size, off_t off);
            static unsigned int sys_pwrite(ObjectHandle h, uintptr_t buffer, size_t size, off_t off);

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
            static unsigned int sys_get_tod(uintptr_t tpstruct);
            static void sys_set_thread_name(ObjectHandle thr, uintptr_t name);
            static unsigned long sys_get_ticks();

            static virt_addr_t sys_mmap(virt_addr_t addr, size_t len, int flags, ObjectHandle fd, off_t offset);

            static void RegisterDefaultSyscalls(SyscallManager& mgr);
        };
    }
}
