/* SPDX-License-Identifier: MIT */

/*
 * kernel/syscall.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/syscall.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/om.h>
#include <infos/kernel/thread.h>
#include <infos/kernel/process.h>
#include <infos/fs/file.h>
#include <infos/fs/directory.h>
#include <infos/util/string.h>
#include <arch/arch.h>

using namespace infos::kernel;
using namespace infos::fs;
using namespace infos::util;

SyscallManager::SyscallManager()
{
	for (int i = 0; i < MAX_SYSCALLS; i++) {
		syscall_table_[i] = nullptr;
	}
}


void SyscallManager::RegisterSyscall(int nr, syscallfn fn)
{
	if (nr < 0 || nr >= MAX_SYSCALLS) return;
	syscall_table_[nr] = fn;
}

unsigned long SyscallManager::InvokeSyscall(int nr, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
{
	if (nr < 0 || nr >= MAX_SYSCALLS) return -1;

	syscallfn fn = syscall_table_[nr];

	if (fn == nullptr) {
		syslog.messagef(LogLevel::DEBUG, "UNHANDLED USER SYSTEM CALL: %lu", nr);
		return -1;
	} else {
		return fn(arg0, arg1, arg2, arg3, arg4, arg5);
	}
}

void DefaultSyscalls::RegisterDefaultSyscalls(SyscallManager& mgr)
{
	mgr.RegisterSyscall(0, (SyscallManager::syscallfn) DefaultSyscalls::sys_nop);
	mgr.RegisterSyscall(1, (SyscallManager::syscallfn) DefaultSyscalls::sys_yield);
	mgr.RegisterSyscall(2, (SyscallManager::syscallfn) DefaultSyscalls::sys_exit);

	mgr.RegisterSyscall(3, (SyscallManager::syscallfn) DefaultSyscalls::sys_open);
	mgr.RegisterSyscall(4, (SyscallManager::syscallfn) DefaultSyscalls::sys_close);
	mgr.RegisterSyscall(5, (SyscallManager::syscallfn) DefaultSyscalls::sys_read);
	mgr.RegisterSyscall(6, (SyscallManager::syscallfn) DefaultSyscalls::sys_write);

	mgr.RegisterSyscall(7, (SyscallManager::syscallfn) DefaultSyscalls::sys_opendir);
	mgr.RegisterSyscall(8, (SyscallManager::syscallfn) DefaultSyscalls::sys_readdir);
	mgr.RegisterSyscall(9, (SyscallManager::syscallfn) DefaultSyscalls::sys_closedir);

	mgr.RegisterSyscall(10, (SyscallManager::syscallfn) DefaultSyscalls::sys_exec);
	mgr.RegisterSyscall(11, (SyscallManager::syscallfn) DefaultSyscalls::sys_wait_proc);
	mgr.RegisterSyscall(12, (SyscallManager::syscallfn) DefaultSyscalls::sys_create_thread);
	mgr.RegisterSyscall(13, (SyscallManager::syscallfn) DefaultSyscalls::sys_stop_thread);
	mgr.RegisterSyscall(14, (SyscallManager::syscallfn) DefaultSyscalls::sys_join_thread);

	mgr.RegisterSyscall(15, (SyscallManager::syscallfn) DefaultSyscalls::sys_usleep);
	mgr.RegisterSyscall(16, (SyscallManager::syscallfn) DefaultSyscalls::sys_get_tod);
	mgr.RegisterSyscall(17, (SyscallManager::syscallfn) DefaultSyscalls::sys_set_thread_name);
	mgr.RegisterSyscall(18, (SyscallManager::syscallfn) DefaultSyscalls::sys_get_ticks);

	mgr.RegisterSyscall(19, (SyscallManager::syscallfn) DefaultSyscalls::sys_pread);
	mgr.RegisterSyscall(20, (SyscallManager::syscallfn) DefaultSyscalls::sys_pwrite);
}

void DefaultSyscalls::sys_nop()
{

}

void DefaultSyscalls::sys_yield()
{
	// syslog.messagef(LogLevel::DEBUG, "YIELD");
}

// TODO: There is no userspace buffer checking done at all.  This really needs to be fixed...

ObjectHandle DefaultSyscalls::sys_open(uintptr_t filename, uint32_t flags)
{
	File *f = sys.vfs().open((const char *) filename, flags);
	if (!f) {
		return KernelObject::Error;
	}

	return sys.object_manager().register_object(Thread::current(), f);
}

unsigned int DefaultSyscalls::sys_close(ObjectHandle h)
{
	File *f = (File *) sys.object_manager().get_object_secure(Thread::current(), h);
	if (!f) {
		return -1;
	}

	f->close();
	return 0;
}

unsigned int DefaultSyscalls::sys_read(ObjectHandle h, uintptr_t buffer, size_t size)
{
	File *f = (File *) sys.object_manager().get_object_secure(Thread::current(), h);
	if (!f) {
		return -1;
	}

	// TODO: Validate 'buffer' etc...
	return f->read((void *) buffer, size);
}

unsigned int DefaultSyscalls::sys_write(ObjectHandle h, uintptr_t buffer, size_t size)
{
	File *f = (File *) sys.object_manager().get_object_secure(Thread::current(), h);
	if (!f) {
		return -1;
	}

	// TODO: Validate 'buffer' etc...
	return f->write((const void *) buffer, size);
}

unsigned int DefaultSyscalls::sys_pread(ObjectHandle h, uintptr_t buffer, size_t size, off_t off)
{
	File *f = (File *) sys.object_manager().get_object_secure(Thread::current(), h);
	if (!f) {
		return -1;
	}

	// TODO: Validate 'buffer' etc...
	return f->pread((void *) buffer, size, off);
}

unsigned int DefaultSyscalls::sys_pwrite(ObjectHandle h, uintptr_t buffer, size_t size, off_t off)
{
	File *f = (File *) sys.object_manager().get_object_secure(Thread::current(), h);
	if (!f) {
		return -1;
	}

	// TODO: Validate 'buffer' etc...
	return f->pwrite((const void *) buffer, size, off);
}

ObjectHandle DefaultSyscalls::sys_opendir(uintptr_t path, uint32_t flags)
{
	Directory *d = sys.vfs().opendir((const char *) path, flags);
	if (!d) return KernelObject::Error;

	return sys.object_manager().register_object(Thread::current(), d);
}

unsigned int DefaultSyscalls::sys_closedir(ObjectHandle h)
{
	Directory *d = (Directory *) sys.object_manager().get_object_secure(Thread::current(), h);
	if (!d) {
		return -1;
	}

	d->close();
	return 0;
}

unsigned int DefaultSyscalls::sys_readdir(ObjectHandle h, uintptr_t buffer)
{
	Directory *d = (Directory *) sys.object_manager().get_object_secure(Thread::current(), h);
	if (!d) {
		return 0;
	}

	DirectoryEntry de;
	if (d->read_entry(de)) {

		struct user_de {
			char name[64];
			unsigned int size;
			int flags;
		};

		user_de *ude = (user_de *) buffer;
		strncpy(ude->name, de.name.c_str(), 63);
		ude->flags = 0;
		ude->size = de.size;

		return 1;
	} else {
		return 0;
	}
}

void DefaultSyscalls::sys_exit(unsigned int rc)
{
	Thread::current().owner().terminate(rc);
}

ObjectHandle DefaultSyscalls::sys_exec(uintptr_t program, uintptr_t args)
{
	Process *p = sys.launch_process((const char *) program, (const char *) args);
	if (!p) {
		return KernelObject::Error;
	}

	return sys.object_manager().register_object(Thread::current(), p);
}

unsigned int DefaultSyscalls::sys_wait_proc(ObjectHandle h)
{
	Process *p = (Process *) sys.object_manager().get_object_secure(Thread::current(), h);
	if (!p) {
		return -1;
	}

	sys.arch().disable_interrupts();
	while (!p->terminated()) {
		sys.arch().enable_interrupts();
		p->state_changed().wait();
	}

	return 0;
}

ObjectHandle DefaultSyscalls::sys_create_thread(uintptr_t entry_point, uintptr_t arg)
{
	Thread& t = Thread::current().owner().create_thread(ThreadPrivilege::User, (Thread::thread_proc_t)entry_point, "other");
	ObjectHandle h = sys.object_manager().register_object(Thread::current(), &t);

	virt_addr_t stack_addr = 0x7fff00000000;
	stack_addr += 0x2000 * (unsigned int) h;

	t.allocate_user_stack(stack_addr, 0x2000);
	//t.owner().vma().allocate_virt()
	t.add_entry_argument((void *) arg);
	t.start();

	return h;
}

unsigned int DefaultSyscalls::sys_stop_thread(ObjectHandle h)
{
	Thread *t;
	if (h == (ObjectHandle) - 1) {
		t = &Thread::current();
	} else {
		t = (Thread *) sys.object_manager().get_object_secure(Thread::current(), h);
	}

	if (!t) {
		return -1;
	}

	t->stop();

	return 0;
}

unsigned int DefaultSyscalls::sys_join_thread(ObjectHandle h)
{
	Thread *t = (Thread *) sys.object_manager().get_object_secure(Thread::current(), h);
	if (!t) {
		return -1;
	}

	sys.arch().disable_interrupts();
	while (!t->stopped()) {
		sys.arch().enable_interrupts();
		t->state_changed().wait();
	}

	return 0;
}

unsigned long DefaultSyscalls::sys_usleep(unsigned long us)
{
	sys.spin_delay(util::Microseconds(us));
	return us;
}

struct userspace_tod_buffer {
	unsigned short seconds, minutes, hours, day_of_month, month, year;
};

unsigned int DefaultSyscalls::sys_get_tod(uintptr_t tpstruct)
{
	auto& tod = sys.time_of_day();

	userspace_tod_buffer *userspace_tod = (userspace_tod_buffer *)tpstruct;
	userspace_tod->day_of_month = tod.day;
	userspace_tod->hours = tod.hours;
	userspace_tod->minutes = tod.minutes;
	userspace_tod->month = tod.month;
	userspace_tod->seconds = tod.seconds;
	userspace_tod->year = tod.year;

	return 0;
}

void DefaultSyscalls::sys_set_thread_name(ObjectHandle h, uintptr_t name)
{
	Thread *t;
	if (h == (ObjectHandle) - 1) {
		t = &Thread::current();
	} else {
		t = (Thread *) sys.object_manager().get_object_secure(Thread::current(), h);
	}

	t->name((const char *)name);
}

unsigned long DefaultSyscalls::sys_get_ticks()
{
	return sys.runtime().time_since_epoch().count();
}
