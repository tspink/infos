/* SPDX-License-Identifier: MIT */

/*
 * kernel/kernel.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/kernel.h>
#include <infos/kernel/sched.h>
#include <infos/kernel/process.h>
#include <infos/kernel/log.h>
#include <infos/util/list.h>
#include <infos/util/cmdline.h>
#include <infos/util/string.h>
#include <infos/fs/file.h>
#include <infos/fs/exec/elf-loader.h>
#include <infos/drivers/block/block-device.h>

#include <arch/arch.h>

using namespace infos::kernel;
using namespace infos::arch;
using namespace infos::util;
using namespace infos::fs;
using namespace infos::drivers;

static char boot_device_name[16];
static char boot_fstype[16];
static char init_program[16];

RegisterCmdLineArgument(BootDevice, "boot-device")
{
	strncpy(boot_device_name, value, sizeof(boot_device_name) - 1);
}

RegisterCmdLineArgument(BootFSType, "boot-fs-type")
{
	strncpy(boot_fstype, value, sizeof(boot_fstype) - 1);
}

RegisterCmdLineArgument(InitProgram, "init")
{
	strncpy(init_program, value, sizeof(init_program) - 1);
}

Kernel infos::kernel::sys(sys_arch);

Kernel::Kernel(Arch& arch)
: _arch(arch),
_object_manager(*this),
_device_manager(*this),
_memory_manager(*this),
_module_manager(*this),
_scheduler(*this),
_vfs(*this)
{

}

void Kernel::start(BottomFn bottom)
{
	syslog.message(LogLevel::INFO, "OK!  Starting the kernel...");
	
	_kernel_process = new Process("kernel", true, (Thread::thread_proc_t) &start_kernel_threadproc_tramp);
	
	_kernel_process->main_thread().add_entry_argument((void *) this);
	_kernel_process->main_thread().add_entry_argument((void *)bottom);
	_kernel_process->start();
		
	syslog.messagef(LogLevel::DEBUG, "Running scheduler");
	scheduler().run();
}

void Kernel::start_kernel_threadproc_tramp(Kernel* kernel, BottomFn bottom)
{
	assert(kernel);
	assert(bottom);
	
	syslog.messagef(LogLevel::INFO, "Now executing in the kernel thread");

	if (!bottom()) {
		syslog.message(LogLevel::FATAL, "Second-half arch-specific kernel initialisation failed");
		arch_abort();
	}

	kernel->start_kernel_threadproc();
	kernel->_kernel_process->terminate(0);
}

void Kernel::start_kernel_threadproc()
{
	if (!vfs().init()) {
		syslog.message(LogLevel::FATAL, "Unable to initialise the FS subsystem");
		arch_abort();
	}
	
	syslog.message(LogLevel::INFO, "Mounting tmpfs root...");
	VFSNode *root = vfs().lookup_node("/");
	
	if (!root->mount("tmpfs", NULL)) {
		syslog.message(LogLevel::FATAL, "Unable to mount tmpfs root filesystem");
		arch_abort();
	}
	
	VFSNode *dev = root->mkdir("dev");
	if (!dev) {
		syslog.message(LogLevel::FATAL, "Unable to create mountpoint for device filesystem");
		arch_abort();
	}
	
	if (!dev->mount("devfs", NULL)) {
		syslog.message(LogLevel::FATAL, "Unable to mount device filesystem");
		arch_abort();
	}
	
	VFSNode *usr = root->mkdir("usr");
	if (!usr) {
		syslog.message(LogLevel::FATAL, "Unable to create mountpoint for userspace filesystem");
		arch_abort();
	}
	
	if (strlen(boot_device_name) == 0) {
		syslog.message(LogLevel::FATAL, "No boot device specified");
		dump_partitions();
		arch_abort();
	}

	Device *boot_device;
	if (!device_manager().try_get_device_by_name(boot_device_name, boot_device)) {
		syslog.messagef(LogLevel::FATAL, "Boot device '%s' not found", boot_device_name);
		dump_partitions();
		arch_abort();
	}

	if (strlen(boot_fstype) == 0) {
		strncpy(boot_fstype, "internal_driver", 16);
	}
	
	util::String bootfstype = boot_fstype;

	syslog.messagef(LogLevel::INFO, "Booting from '%s'...", boot_device->name().c_str());	
	
	syslog.messagef(LogLevel::IMPORTANT, "*** USING FILE-SYSTEM DRIVER: %s", bootfstype.c_str());
	
	if (!usr->mount(bootfstype, boot_device)) {
		syslog.message(LogLevel::FATAL, "Unable to mount root filesystem");
		arch_abort();
	}
	
	if (!launch_process(init_program, "")) {
		syslog.messagef(LogLevel::FATAL, "Unable to launch init=%s", init_program);
		arch_abort();
	}
}

bool Kernel::early_init(const char* cmdline)
{
	return this->cmdline().parse(cmdline);
}

void Kernel::update_runtime(Nanoseconds ticks)
{
	_runtime += ticks;
}

Process *Kernel::launch_process(const String& path, const String& cmdline)
{
	syslog.messagef(LogLevel::DEBUG, "Launching application: '%s' '%s'", path.c_str(), cmdline.c_str());
	File *image = vfs().open(path, 0);
	if (!image) {
		syslog.message(LogLevel::ERROR, "Process not found");
		return NULL;
	}
		
	uint8_t hdr[16];
	image->read(hdr, sizeof(hdr));
	
	if (hdr[0] == 0x7f && hdr[1] == 'E' && hdr[2] == 'L' && hdr[3] == 'F') {
		exec::ElfLoader *loader = new exec::ElfLoader(*image);
		Process *np = loader->load(cmdline);
		if (!np) {
			delete loader;
			delete image;
			
			return NULL;
		}
		
		syslog.messagef(LogLevel::DEBUG, "Starting process... %p", np->main_thread().context().native_context->rdi);
		np->start();
		delete loader;
		delete image;
		
		return np;
	} else {
		delete image;
		return NULL;
	}
}

void Kernel::spin_delay(util::Nanoseconds ns)
{
	KernelRuntimeClock::Timepoint target = sys.runtime() + ns;
	while (sys.runtime() < target) asm volatile("pause");
}

void Kernel::dump_partitions()
{
	syslog.message(LogLevel::INFO, "Available partitions:");
	
	for (auto device : device_manager().devices()) {
		if (device.value->device_class().is(infos::drivers::block::BlockDevice::BlockDeviceClass)) {
			syslog.messagef(LogLevel::INFO, "  %s", device.value->name().c_str());
		}
	}
}
