/* SPDX-License-Identifier: MIT */

/*
 * include/arch/x86/init.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/define.h>

namespace infos {
	namespace kernel {
		class ComponentLog;
	}
	
	namespace arch {
		namespace x86 {
			extern bool platform_init(void);
			extern bool mm_init(void);
			extern bool mm_pf_init(void);
			extern bool cpu_init(void);
			extern bool modules_init(void);
			extern bool sched_init(void);
			
			extern bool timer_init(void);
			extern bool console_init(void);
			extern bool activate_console(void);
			
			extern bool devices_init(void);
			
			extern kernel::ComponentLog x86_log;
		}
	}
}
