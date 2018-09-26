/* SPDX-License-Identifier: MIT */

/*
 * include/arch/x86/pio.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/define.h>

namespace infos
{
	namespace arch
	{
		namespace x86
		{
			inline void __outb(int port, uint8_t data)
			{
				asm volatile("outb %%al, %%dx" :: "a"(data), "d"((uint16_t)port));
			}

			inline void __outw(int port, uint16_t data)
			{
				asm volatile("outw %%ax, %%dx" :: "a"(data), "d"((uint16_t)port));
			}
			
			inline void __outl(int port, uint32_t data)
			{
				asm volatile("outl %%eax, %%dx" :: "a"(data), "d"((uint16_t)port));
			}
			
			inline uint8_t __inb(int port)
			{
				uint8_t ret;
				asm volatile("inb %%dx, %%al" : "=a"(ret) : "d"((uint16_t)port));
				return ret;
			}

			inline uint16_t __inw(int port)
			{
				uint16_t ret;
				asm volatile("inw %%dx, %%ax" : "=a"(ret) : "d"((uint16_t)port));
				return ret;
			}

			inline uint32_t __inl(int port)
			{
				uint32_t ret;
				asm volatile("inl %%dx, %%eax" : "=a"(ret) : "d"((uint16_t)port));
				return ret;
			}
			
			inline void __insw(uint16_t port, uintptr_t buffer, size_t count)
			{
				asm volatile("cld\n\trepnz insw" : "=D"(buffer), "=c"(count)
				: "d"(port), "0"(buffer), "1"(count)
				: "memory", "cc" );
			}
			
			inline void __insl(uint16_t port, uintptr_t buffer, size_t count)
			{
				asm volatile("cld\n\trepnz insl" : "=D"(buffer), "=c"(count)
				: "d"(port), "0"(buffer), "1"(count)
				: "memory", "cc" );
			}
		}
	}
}
