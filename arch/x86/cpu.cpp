/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/cpu.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/init.h>
#include <arch/x86/cpu.h>
#include <arch/x86/msr.h>
#include <arch/x86/acpi/acpi.h>

#include <infos/define.h>

using namespace infos::kernel;
using namespace infos::arch;
using namespace infos::util;
using namespace infos::mm;
using namespace infos::arch::x86;
using namespace infos::drivers::timer;
using namespace infos::drivers::irq;

// CPU Logging Component
infos::kernel::ComponentLog infos::arch::x86::cpu_log(infos::kernel::syslog, "cpu");

/**
 * Initialises the CPU.
 * @return Returns TRUE if the CPU was successfully initialised, or FALSE otherwise.
 */
bool infos::arch::x86::cpu_init()
{
    // todo: model this properly

    // Determine the LAPIC base address.
    uint64_t lapic_base = __rdmsr(MSR_APIC_BASE) & ~0xfff;
    if (!lapic_base) {
        cpu_log.message(LogLevel::ERROR, "Invalid LAPIC base address");
        return false;
    }

    // Determine the IOAPIC base address.
    uint32_t ioapic_base = infos::arch::x86::acpi::acpi_get_ioapic_base();

    // Print out some information about the memory-mapped location of these
    // structures.
    cpu_log.messagef(LogLevel::DEBUG, "LAPIC base=%lx, IOAPIC base=%lx", lapic_base, ioapic_base);

    // Create and register a PIT (Programmable Interrupt Timer), used here for spin delay
    PIT *pit = new PIT();
    if (!sys.device_manager().register_device(*pit))
        return false;

    // Create and register the BSP LAPIC object.
    LAPIC *lapic = new LAPIC(pa_to_vpa(lapic_base));
    if (!sys.device_manager().register_device(*lapic))
        return false;

    // Create and register an IOAPIC object.
    IOAPIC *ioapic = new IOAPIC(pa_to_vpa(ioapic_base));
    if (!sys.device_manager().register_device(*ioapic))
        return false;

    Core* cores = infos::arch::x86::acpi::acpi_get_cores();
    uint8_t num_cores = infos::arch::x86::acpi::acpi_get_num_cores();

    for (int i = 0; i < num_cores; i++) {
        // skip BSP and error cores
        if (cores[i].get_state() == Core::core_state::OFFLINE) {
            // start the core!
            start_core(&cores[i], lapic, pit);
        }
    }

    // Once all cores have been initialised, remove the zero
    // page mapping used for AP trampoline code
    mm_remove_multicore_mapping();

    return true;
}

extern char _MPSTARTUP_START;
extern char _MPSTARTUP_END;
extern unsigned long mpcr3, mpstack;
extern unsigned char mpready;

void infos::arch::x86::start_core(Core* core, LAPIC* lapic, PIT* pit) {
    uint8_t processor_id = core->get_processor_id();

    // Copy AP trampoline code onto start page
    uint8_t* start_page = (uint8_t*) pa_to_vpa(0);
    size_t mpstartup_size = (uint64_t)&_MPSTARTUP_END - (uint64_t)&_MPSTARTUP_START;
    memcpy(start_page, (const void *)(pa_to_vpa((uintptr_t) &_MPSTARTUP_START)), mpstartup_size);

    // Insert CR3 value
    uint64_t this_cr3;
    asm volatile("mov %%cr3, %0" : "=r"(this_cr3));
    size_t mpcr3_offset = (uint64_t)&mpcr3 - (uint64_t)&_MPSTARTUP_START;
    *((uint64_t *)pa_to_vpa(mpcr3_offset)) = this_cr3;		// HACK: ONLY WORKS IF BASE PFN=0

    // Insert RSP value
    size_t mprsp_offset = (uint64_t)&mpstack - (uint64_t)&_MPSTARTUP_START;
    PageDescriptor* pgd = sys.mm().pgalloc().alloc_pages(0);
    *((uint64_t *)pa_to_vpa(mprsp_offset)) = sys.mm().pgalloc().pgd_to_kva(pgd);

    asm volatile("mfence" ::: "memory");

    // Get MPREADY flag
    size_t mpready_offset = (uint64_t)&mpready - (uint64_t)&_MPSTARTUP_START;
    volatile uint8_t *ready_flag = (volatile uint8_t *)pa_to_vpa(mpready_offset);
    *ready_flag = 0;

    // send init and wait 10ms
    cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "sending init to core %u", processor_id);
    lapic->send_remote_init(processor_id, 0);
    pit->spin_delay(10000000);

    // send sipi and wait 1ms
    cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "sending sipi to core %u", processor_id);
    lapic->send_remote_sipi(processor_id, 0);
    pit->spin_delay(1000000);

    if (!*ready_flag) {
        // send second sipi and wait 1s
        cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "resending sipi to core %u", processor_id);
        lapic->send_remote_sipi(processor_id,0);
        pit->spin_delay(1000000000);
    }

    if (!*ready_flag) {
        cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "core %u error, skipping", processor_id);
    } else {
        core->set_state(Core::core_state::ONLINE);
        cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "core %u ready", processor_id);
    }
}

/**
 * Constructs a new X86CPU object.
 */
X86CPU::X86CPU()
{

}
