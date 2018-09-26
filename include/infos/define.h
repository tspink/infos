/* SPDX-License-Identifier: MIT */

/*
 * include/define.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

typedef unsigned char			uint8_t;
typedef unsigned short			uint16_t;
typedef unsigned int			uint32_t;
typedef unsigned long long int	uint64_t;

typedef signed char				int8_t;
typedef signed short			int16_t;
typedef signed int				int32_t;
typedef signed long long int	int64_t;

typedef unsigned long	size_t;
typedef unsigned long	off_t;
typedef unsigned long	uintptr_t;
typedef signed long		intptr_t;

typedef signed long int		intmax_t;
typedef unsigned long int	uintmax_t;

typedef uintptr_t		phys_addr_t;
typedef uintptr_t		virt_addr_t;
typedef uint64_t		pfn_t;

#include <infos/assert.h>

#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

#define __weak __attribute__((weak))
#define __packed __attribute__((packed))
#define __noreturn __attribute__((noreturn))
#define __pure __attribute__((pure))
#define __aligned(__n) __attribute__((aligned(__n)))
#define __section(__n) __attribute__((section(__n)))
#define __init_priority(__n) __attribute__((init_priority(__n)))
#define __unreachable() __builtin_unreachable()

#define ARRAY_SIZE(__arr) (sizeof(__arr) / sizeof(__arr[0]))

#define offsetof(st, m) __builtin_offsetof(st, m)

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define NULL nullptr

#define KB(__val) (__val >> 10)
#define MB(__val) (KB(__val) >> 10)
#define GB(__val) (MB(__val) >> 10)

#define KERNEL_VMEM_START	((uintptr_t)0xFFFFFFFF80000000u)
#define KERNEL_VMEM_END		((uintptr_t)0xFFFFFFFFFFFFFFFFu)
#define KERNEL_VMEM_SIZE	(KERNEL_VMEM_END - KERNEL_VMEM_START + 1)

#define PMEM_VA_START		((uintptr_t)0xFFFF800000000000)
#define PMEM_VA_END			((uintptr_t)0xFFFF800100000000)
#define PMEM_VA_SIZE		(PMEM_VA_END - PMEM_VA_START)

#define STRINGIFY(__N) _STRINGIFY(__N)
#define _STRINGIFY(__N) #__N

#define __page_bits 12
#define __page_size (1 << __page_bits)
#define __page_offset(__addr) ((__addr) & (__page_size - 1))
#define __page_base(__addr) ((__addr) & ~(__page_size - 1))
#define __page_index(__addr) ((__addr) >> __page_bits)

/**
 * Converts a physical address into a kernel virtual address.
 * @param pa The physical address to convert.
 * @return The kernel virtual address of the given physical address.
 */
static inline virt_addr_t pa_to_kva(phys_addr_t pa)
{
	assert(pa < KERNEL_VMEM_SIZE);
	return KERNEL_VMEM_START + pa;
}

static inline phys_addr_t kva_to_pa(virt_addr_t va)
{
	assert((va >= KERNEL_VMEM_START) && (va <= KERNEL_VMEM_END));
	return va - KERNEL_VMEM_START;
}

static inline virt_addr_t pa_to_vpa(phys_addr_t pa)
{
	assert(pa < PMEM_VA_SIZE);
	return PMEM_VA_START + pa;
}

static inline phys_addr_t vpa_to_pa(virt_addr_t va)
{
	assert((va >= PMEM_VA_START) && (va < PMEM_VA_END));
	return va - PMEM_VA_START;
}

static inline pfn_t pa_to_pfn(phys_addr_t pa)
{
	return (pfn_t)(pa >> __page_bits);
}

static inline phys_addr_t pfn_to_pa(pfn_t pfn)
{
	return (phys_addr_t)(pfn << __page_bits);
}

#define __align_up(__value, __alignment) (__value + ((__value % __alignment == 0) ? 0 : (__alignment - (__value % __alignment))))
#define __align_up_page(__value) __align_up(__value, __page_size)

#define __cxp
//#define __cxp constexpr

__pure static inline __cxp uint64_t __log2ceil(uint64_t x)
{
	if (!x) return 0;

	uint64_t index = __builtin_clzll(x);
	return (63 - index) + ((x & (x-1)) ? 1 : 0);
}

__pure static inline __cxp uint64_t __min(uint64_t x, uint64_t y)
{
	return x < y ? x : y;
}

__pure static inline __cxp uint64_t __max(uint64_t x, uint64_t y)
{
	return x < y ? y : x;
}

extern "C" void arch_abort() __noreturn;
