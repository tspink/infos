/* SPDX-License-Identifier: MIT */

/*
 * include/arch/x86/cpuid.h
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

			struct CPUID {
				uint64_t rax, rbx, rcx, rdx;
			};

#define CPUID_GETVENDOR			0x00000000
#define CPUID_GET_FEATURES		0x00000001
#define CPUID_GET_EX_FEATURES	0x80000001

			static inline CPUID __cpuid(uint64_t rax) {
				CPUID ret;
				asm volatile("cpuid" : "=a"(ret.rax), "=b"(ret.rbx), "=c"(ret.rcx), "=d"(ret.rdx) : "a"(rax));

				return ret;
			}

			namespace CPUIDFeatures {

				enum CPUIDFeaturesRCX {
					SSE3 = 1 << 0,
					PCLMUL = 1 << 1,
					DTES64 = 1 << 2,
					MONITOR = 1 << 3,
					DS_CPL = 1 << 4,
					VMX = 1 << 5,
					SMX = 1 << 6,
					EST = 1 << 7,
					TM2 = 1 << 8,
					SSSE3 = 1 << 9,
					CID = 1 << 10,
					FMA = 1 << 12,
					CX16 = 1 << 13,
					ETPRD = 1 << 14,
					PDCM = 1 << 15,
					DCA = 1 << 18,
					SSE4_1 = 1 << 19,
					SSE4_2 = 1 << 20,
					x2APIC = 1 << 21,
					MOVBE = 1 << 22,
					POPCNT = 1 << 23,
					AES = 1 << 25,
					XSAVE = 1 << 26,
					OSXSAVE = 1 << 27,
					AVX = 1 << 28,
				};

				enum CPUIDFeaturesRDX {
					FPU = 1 << 0,
					VME = 1 << 1,
					DE = 1 << 2,
					PSE = 1 << 3,
					TSC = 1 << 4,
					MSR = 1 << 5,
					PAE = 1 << 6,
					MCE = 1 << 7,
					CX8 = 1 << 8,
					APIC = 1 << 9,
					SEP = 1 << 11,
					MTRR = 1 << 12,
					PGE = 1 << 13,
					MCA = 1 << 14,
					CMOV = 1 << 15,
					PAT = 1 << 16,
					PSE36 = 1 << 17,
					PSN = 1 << 18,
					CLF = 1 << 19,
					DTES = 1 << 21,
					ACPI = 1 << 22,
					MMX = 1 << 23,
					FXSR = 1 << 24,
					SSE = 1 << 25,
					SSE2 = 1 << 26,
					SS = 1 << 27,
					HTT = 1 << 28,
					TM1 = 1 << 29,
					IA64 = 1 << 30,
					PBE = 1 << 31
				};
				
				enum CPUIDExFeaturesRCX {
					LAHFLM = 1 << 0
				};

				enum CPUIDExFeaturesRDX {
					PDPE1GB = 1 << 26
				};

				struct CPUIDFeatures {
					CPUIDFeaturesRCX rcx;
					CPUIDFeaturesRDX rdx;
				};
				
				struct CPUIDExFeatures {
					CPUIDExFeaturesRCX rcx;
					CPUIDExFeaturesRDX rdx;
				};
			}

			static inline CPUIDFeatures::CPUIDFeatures cpuid_get_features() {
				CPUIDFeatures::CPUIDFeatures ret;

				ret.rcx = (CPUIDFeatures::CPUIDFeaturesRCX) __cpuid(CPUID_GET_FEATURES).rcx;
				ret.rdx = (CPUIDFeatures::CPUIDFeaturesRDX) __cpuid(CPUID_GET_FEATURES).rdx;

				return ret;
			}
			
			static inline CPUIDFeatures::CPUIDExFeatures cpuid_get_ex_features() {
				CPUIDFeatures::CPUIDExFeatures ret;

				ret.rcx = (CPUIDFeatures::CPUIDExFeaturesRCX) __cpuid(CPUID_GET_EX_FEATURES).rcx;
				ret.rdx = (CPUIDFeatures::CPUIDExFeaturesRDX) __cpuid(CPUID_GET_EX_FEATURES).rdx;

				return ret;
			}
		}
	}
}
