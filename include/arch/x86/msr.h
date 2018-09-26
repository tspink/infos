/* SPDX-License-Identifier: MIT */

/*
 * include/arch/x86/msr.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/define.h>

namespace infos {
	namespace arch {
		namespace x86 {
#define MSR_STAR 0xc0000081
#define MSR_LSTAR 0xc0000082
#define MSR_CSTAR 0xc0000083
#define MSR_SFMASK 0xc0000084

#define MSR_APIC_BASE 0x1b
			
#define MSR_FS_BASE 0xc0000100
#define MSR_GS_BASE 0xc0000101

			static inline void __wrmsr(uint32_t msr_id, uint64_t msr_value) {
				uint32_t low = msr_value & 0xffffffff;
				uint32_t high = (msr_value >> 32);

				asm volatile ( "rex.b wrmsr" : : "c" (msr_id), "a" (low), "d" (high));
			}

			static inline uint64_t __rdmsr(uint32_t msr_id) {
				uint32_t low, high;

				asm volatile("rex.b rdmsr" : "=a"(low), "=d"(high) : "c" (msr_id));
				return (uint64_t) low | (((uint64_t) high) << 32);
			}
		}
	}
}
