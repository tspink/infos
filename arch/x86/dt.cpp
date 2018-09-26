/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/dt.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/dt.h>
#include <arch/x86/irq-entry-points.h>
#include <infos/util/string.h>

using namespace infos::arch::x86;

// GDT, IDT and TSS instantiation -- make sure they're aligned nicely.
__aligned(16) GDT infos::arch::x86::gdt;
__aligned(16) IDT infos::arch::x86::idt;
__aligned(16) TSS infos::arch::x86::tss;

/**
 * Initialises the Global Descriptor Table
 * @return Returns true if initialisation was successful, false otherwise.
 */
bool GDT::init()
{
	// Clear the GDT.
	erase();
	
	// Initial null segment.
	if (!add_null()) return false;					// 0
	
	// Kernel code and data segments.
	if (!add_code_segment(0)) return false;			// 8
	if (!add_data_segment(0)) return false;			// 10

	// User code and data segments.
	if (!add_code_segment(3)) return false;			// 18
	if (!add_data_segment(3)) return false;			// 20
	
	// TSS
	if (!add_tss((void *)tss.__tss, sizeof(tss.__tss))) return false;	// 28
	
	return reload();
}

/**
 * Reloads the Global Descriptor Table after modification of the entries.
 * @return Returns true if the reload was successful, false otherwise.
 */
bool GDT::reload()
{
	// Load the address of the GDT into the GDTR
	const GDTPointer ptr = { .length = (uint16_t)(_current * 8u), .ptr = __gdt };
	asm volatile("lgdt %0" :: "m"(ptr));
	
	// Reload segment registers.
	asm volatile("mov %0, %%ds" :: "a"(0x10));
	asm volatile("mov %0, %%es" :: "a"(0x10));
	asm volatile("mov %0, %%ss" :: "a"(0x10));
	
	// TODO: Think about reloading the CS register, which needs to
	// be done with a longjmp.  This was previously done in the assembly
	// start-up code, with the temporary GDT, but it should probably be
	// done with the new GDT.
	
	return true;
}

/**
 * Retrieves the base address of the current GDT
 * @return Returns the base address present in the GDTR
 */
uintptr_t GDT::get_ptr()
{
	GDTPointer ptr;
	asm volatile("sgdt %0" : "=m"(ptr));
	
	return (uintptr_t)ptr.ptr;
}


/**
 * Erases any existing entries in the GDT.
 */
void GDT::erase()
{
	// Just reset the _current pointer, so that adding entries will overwrite
	// existing ones.
	_current = 0;
}

/**
 * Inserts a NULL segment descriptor into the GDT.
 * @return Returns true if the insertion was successful, false otherwise.
 */
bool GDT::add_null()
{
	if (_current >= MAX_NR_GDT_ENTRIES) return false;
	
	__gdt[_current++] = 0;
	return true;
}

/**
 * Inserts a 64-bit code segment descriptor into the GDT.
 * @return Returns true if the insertion was successful, false otherwise.
 */
bool GDT::add_code_segment(uint8_t dpl)
{
	if (_current >= MAX_NR_GDT_ENTRIES) return false;
	
	CodeSegmentDescriptor desc(dpl);
	
	__gdt[_current++] = desc.bits;
	return true;
}

/**
 * Inserts a data segment descriptor into the GDT.
 * @param dpl The privilege level that this descriptor describes.
 * @return Returns true if the insertion was successful, false otherwise.
 */
bool GDT::add_data_segment(uint8_t dpl)
{
	if (_current >= MAX_NR_GDT_ENTRIES) return false;
	
	DataSegmentDescriptor desc(dpl);
	
	__gdt[_current++] = desc.bits;
	return true;
}

/**
 * Inserts a TSS descriptor into the GDT.
 * @param ptr The linear address of the TSS structure.
 * @return Returns true if the insertion was successful, false otherwise.
 */
bool GDT::add_tss(void* ptr, size_t size)
{
	// Need -1 here, because the TSS descriptor takes up TWO GDT slots.
	if (_current >= MAX_NR_GDT_ENTRIES - 1) return false;

	TSSDescriptor desc(ptr, size);
	
	__gdt[_current++] = desc.bits_low;
	__gdt[_current++] = desc.bits_high;
	
	return true;
}

/**
 * Initialises the Interrupt Descriptor Table.
 * @return Returns true if initialisation was successful, false otherwise.
 */
bool IDT::init()
{
	// Clear out the IDT
	infos::util::bzero(__idt, sizeof(__idt));
	_max_index = 0;

	// The IDT entries are initialised externally, by the IRQ manager.
	
	// Reload the IDT register.
	return reload();
}

/**
 * Reloads the IDT register after any changes to the IDT.
 * @return 
 */
bool IDT::reload()
{
	const IDTPointer ptr = { .length = (uint16_t)((_max_index + 1) * 16u), .ptr = __idt };
	asm volatile("lidt %0" :: "m"(ptr));
	
	return true;
}

/**
 * Retrieves the base pointer of the current IDT
 * @return Returns the base address present in the IDTR
 */
uintptr_t IDT::get_ptr()
{
	IDTPointer ptr;
	asm volatile("sidt %0" : "=m"(ptr));
	
	return (uintptr_t)ptr.ptr;
}

/**
 * Registers an interrupt gate in the IDT, at the given index.
 * @param index The index at which to register the interrupt gate.
 * @param addr The address of the interrupt handling routine.
 * @param seg The code-segment selector for the interrupt handling routine.
 * @param dpl The privilege level at which the interrupt gate can be invoked.
 * @return Returns true if the insertion succeeded, false otherwise.
 */
bool IDT::register_interrupt_gate(int index, uintptr_t addr, uint16_t seg, uint8_t dpl)
{
	if (index >= MAX_NR_IDT_ENTRIES) return false;
	
	InterruptGateDescriptor desc(addr, seg, dpl);
	
	__idt[index].low = desc.bits_low;
	__idt[index].high = desc.bits_high;
	
	if (index > _max_index) {
		_max_index = index;
	}
	
	return true;
}

/**
 * Registers a trap gate in the IDT, at the given index.
 * @param index The index at which to register the trap gate.
 * @param addr The address of the trap handling routine.
 * @param seg The code-segment selector for the trap handling routine.
 * @param dpl The privilege level at which the trap gate can be invoked.
 * @return Returns true if the insertion succeeded, false otherwise.
 */
bool IDT::register_trap_gate(int index, uintptr_t addr, uint16_t seg, uint8_t dpl)
{
	if (index >= MAX_NR_IDT_ENTRIES) return false;
	
	TrapGateDescriptor desc(addr, seg, dpl);
	
	__idt[index].low = desc.bits_low;
	__idt[index].high = desc.bits_high;
	
	if (index > _max_index) {
		_max_index = index;
	}
	
	return true;
}

extern char _STACK_END;

/**
 * Initialises the TSS by loading the task register (TR) with the selector of
 * the TSS descriptor in the GDT.
 * @param sel The selector of the TSS descriptor in the GDT.
 * @return Returns true if initialisation was successful, or false otherwise.
 */
bool TSS::init(uint16_t sel)
{
	set_kernel_stack(0);
	
	asm volatile("ltr %0" :: "r"(sel));
	return true;
}

void TSS::set_kernel_stack(uintptr_t stack)
{
	uint64_t *fields = (uint64_t *)((uintptr_t)__tss + 4);
	fields[0] = (uint64_t)stack;
}

/**
 * Interrogates the task register (TR) to determine the selector that references
 * the TSS descriptor in the GDT.
 * @return The selector that references the TSS descriptor in the GDT.
 */
uint16_t TSS::get_sel()
{
	uint16_t sel;
	asm volatile("str %0" : "=r"(sel));
	
	return sel;
}
