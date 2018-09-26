/* SPDX-License-Identifier: MIT */

/*
 * include/arch/x86/dt.h
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
			struct DataSegmentDescriptor {
				DataSegmentDescriptor(uint8_t dpl) 
				: limit_low(0),
				base_low(0),
				base_middle(0),
				A(0),
				W(1),
				E(0),
				ra_0_0(0),
				ra_1_1(1),
				DPL(dpl),
				P(1),
				limit_high(0),
				AVL(0),
				ra_0_2(0),
				B(1),
				G(0),
				base_high(0) { }
				
				union {
					uint64_t bits;
					struct {
						uint16_t limit_low;
						uint16_t base_low;
						uint8_t base_middle;

						uint8_t A : 1;
						uint8_t W : 1;
						uint8_t E : 1;
						uint8_t ra_0_0 : 1;

						uint8_t ra_1_1 : 1;

						uint8_t DPL : 2;
						uint8_t P : 1;

						uint8_t limit_high : 4;

						uint8_t AVL : 1;
						uint8_t ra_0_2 : 1;
						uint8_t B : 1;
						uint8_t G : 1;

						uint8_t base_high;
					} __packed;
				};
			} __packed;
			
			struct CodeSegmentDescriptor {
				CodeSegmentDescriptor(uint8_t dpl) 
					: ra_0_1(0),
					ra_0_2(0),
					A(0),
					R(1),
					C(0),
					ra_1_3(1),
					ra_1_4(1),
					DPL(dpl),
					P(1),
					ra_0_5(0),
					AVL(0),
					L(1),
					D(0),
					G(0),
					ra_0_6(0)
				{ }
				
				union {
					uint64_t bits;
					struct {
						uint32_t ra_0_1;
						uint8_t ra_0_2;

						uint8_t A : 1;
						uint8_t R : 1;
						uint8_t C : 1;

						uint8_t ra_1_3 : 1;
						uint8_t ra_1_4 : 1;

						uint8_t DPL : 2;
						uint8_t P : 1;

						uint8_t ra_0_5 : 4;

						uint8_t AVL : 1;
						uint8_t L : 1;
						uint8_t D : 1;
						uint8_t G : 1;

						uint8_t ra_0_6;
					} __packed;
				};
			} __packed;
			
			struct TSSDescriptor {
				TSSDescriptor(void *ptr, size_t size) 
					: limit_low((size & 0xffff)),
					base_low(((uintptr_t)ptr) & 0xffff),
					base_middle((((uintptr_t)ptr) >> 16) & 0xff),
					Type(9),
					ra_0_0(0),
					DPL(0),
					P(1),
					limit_high((size >> 16) & 0xf),
					AVL(0),
					ra_0_1(0),
					ra_1_2(1),
					G(0),
					base_high((((uintptr_t)ptr) >> 24) & 0xff),
					base_xhigh(((uintptr_t)ptr) >> 32)
				{ }
				
				union {
					struct { uint64_t bits_low, bits_high; } __packed;
					struct {
						uint16_t limit_low;
						uint16_t base_low;
						uint8_t base_middle;

						uint8_t Type : 4;
						uint8_t ra_0_0 : 1;

						uint8_t DPL : 2;
						uint8_t P : 1;

						uint8_t limit_high : 4;

						uint8_t AVL : 1;
						uint8_t ra_0_1 : 1;
						uint8_t ra_1_2 : 1;
						uint8_t G : 1;

						uint8_t base_high;
						
						uint32_t base_xhigh;
						uint32_t reserved;
					} __packed;
				};
			} __packed;
			
			struct InterruptGateDescriptor {
				InterruptGateDescriptor(uintptr_t offset, uint16_t seg, uint8_t dpl)
					: offset_low(offset & 0xffffu),
				segment_selector(seg),
				ist(0),
				ra_1c0_0(0x1c0),
				DPL(dpl),
				P(1),
				offset_mid((offset >> 16) & 0xffffu),
				offset_high((offset >> 32) & 0xffffffffu) { }
				
				union {
					struct { uint64_t bits_low, bits_high; } __packed;
					struct {
						uint16_t offset_low;
						uint16_t segment_selector;
						uint8_t ist : 3;
						uint16_t ra_1c0_0 : 10;
						uint8_t DPL : 2;
						uint8_t P : 1;
						uint16_t offset_mid;
						uint32_t offset_high;
						uint32_t ra_0_1;
					} __packed;
				};
			} __packed;
			
			struct TrapGateDescriptor {
				TrapGateDescriptor(uintptr_t offset, uint16_t seg, uint8_t dpl)
					: offset_low(offset & 0xffffu),
				segment_selector(seg),
				ist(0),
				ra_1e0_0(0x1e0),
				DPL(dpl),
				P(1),
				offset_mid((offset >> 16) & 0xffffu),
				offset_high((offset >> 32) & 0xffffffffu) { }
				
				union {
					struct { uint64_t bits_low, bits_high; } __packed;
					struct {
						uint16_t offset_low;
						uint16_t segment_selector;
						uint8_t ist : 2;
						uint16_t ra_1e0_0 : 10;
						uint8_t DPL : 2;
						uint8_t P : 1;
						uint16_t offset_mid;
						uint32_t offset_high;
						uint32_t ra_0_1;
					} __packed;
				};
			} __packed;

			struct GDTPointer {
				uint16_t length;
				const void *ptr;
			} __packed;
						
			
			struct IDTPointer {
				uint16_t length;
				const void *ptr;
			} __packed;
			
			class DT {
			public:
				virtual bool init() = 0;
				virtual bool reload() = 0;
				virtual uintptr_t get_ptr() = 0;
			};
			
#define MAX_NR_GDT_ENTRIES	16
			
			class GDT : public DT {
			public:
				bool init() override;
				bool reload() override;
				uintptr_t get_ptr() override;

				void erase();
				
				bool add_null();
				bool add_code_segment(uint8_t dpl);
				bool add_data_segment(uint8_t dpl);
				bool add_tss(void *ptr, size_t size);
				
			private:
				uint8_t _current;
				uint64_t __aligned(16) __gdt[MAX_NR_GDT_ENTRIES];
			};

#define MAX_NR_IDT_ENTRIES	256

			class IDT : public DT {
			public:
				bool init() override;
				bool reload() override;
				uintptr_t get_ptr() override;
				
				bool register_interrupt_gate(int index, uintptr_t addr, uint16_t seg, uint8_t dpl);
				bool register_trap_gate(int index, uintptr_t addr, uint16_t seg, uint8_t dpl);
				
			private:
				uint8_t _max_index;
				struct { uint64_t low, high; } __packed __aligned(16) __idt[MAX_NR_IDT_ENTRIES];
			};
						
			class TSS {
				friend class GDT;
				
			public:
				bool init(uint16_t sel);
				uint16_t get_sel();
				
				void set_kernel_stack(uintptr_t stack);

			private:
				uint64_t __tss[26];				
			};
			
			extern GDT gdt;
			extern IDT idt;
			extern TSS tss;
		}
	}
}
