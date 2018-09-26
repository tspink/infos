/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/modules.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/init.h>
#include <arch/x86/multiboot.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/log.h>

using namespace infos::arch::x86;
using namespace infos::kernel;

bool infos::arch::x86::modules_init()
{
	for (unsigned int i = 0; i < multiboot_info_structure->mods_count; i++) {
		struct multiboot_module_entry *module_entry = (struct multiboot_module_entry *)pa_to_vpa(multiboot_info_structure->mods_addr + (sizeof(struct multiboot_module_entry) * i));
		
		uintptr_t module_start_va = pa_to_vpa(module_entry->mod_start);
		uintptr_t module_end_va = pa_to_vpa(module_entry->mod_end);
		size_t module_size = module_end_va - module_start_va;
		
		x86_log.messagef(LogLevel::INFO, "Loading module: %s @ %p", module_entry->cmdline, module_start_va);
		if (!sys.module_manager().LoadModule((void *)module_start_va, module_size)) {
			x86_log.message(LogLevel::ERROR, "Error loading module");
			return false;
		}
	}
	
	return true;
}
