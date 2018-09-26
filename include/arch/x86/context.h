/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/define.h>

extern "C" {
	struct X86Context
	{
		uint64_t previous_context;
		uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
		uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
		uint64_t extra, rip, cs, rflags, rsp, ss;
	} __packed;
}
