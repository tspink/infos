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
#include <arch/x86/cpuid.h>
#include <infos/drivers/timer/lapic-timer.h>

using namespace infos::kernel;
using namespace infos::arch;
using namespace infos::util;
using namespace infos::mm;
using namespace infos::arch::x86;
using namespace infos::drivers::timer;
using namespace infos::drivers::irq;

// CPU Logging Component
infos::kernel::ComponentLog infos::arch::x86::cpu_log(infos::kernel::syslog, "cpu");

static bool init_smp;

RegisterCmdLineArgument(SMP, "smp") {
    if (strncmp("yes", value, 3) == 0) {
        init_smp = true;
    }
}

/**
 * Initialises the CPU.
 * @return Returns TRUE if the CPU was successfully initialised, or FALSE otherwise.
 */
bool infos::arch::x86::cpu_init()
{
    LAPIC *lapic;
    if (!sys.device_manager().try_get_device_by_class(LAPIC::LAPICDeviceClass, lapic))
        return false;

    PIT *pit;
    if (!sys.device_manager().try_get_device_by_class(PIT::PITDeviceClass, pit))
        return false;



    // Messy hack: we don't know the number of cores before the static cores array is created,
    // so we declare size 32 (max number of cores supported) and initialise all as nullptrs
    // then reassign indices of cores that exist with core objects
    if (init_smp) {
        List<Core*> cores = sys.device_manager().cores();

        for (Core* core : cores) {
            if (core->get_state() == Core::core_state::OFFLINE) {
                // Start the core!
                cpu_log.messagef(LogLevel::DEBUG, "starting core %u", core->get_lapic_id());
                start_core(core, lapic, pit);
            }
        }

//        Core** cores = Core::get_cores();
//
//        for (int i = 0; i < 32; i++) {
//            Core* core = *(cores+i);
//            if (core == nullptr) continue; // reached end of core array
//
//            if (core->get_state() == Core::core_state::OFFLINE) {
//                // Start the core!
//                cpu_log.messagef(LogLevel::DEBUG, "starting core %u", core->get_lapic_id());
//                start_core(core, lapic, pit);
//            }
//        }

        // Once all cores have been initialised, remove the zero
        // page mapping used for AP trampoline code
        mm_remove_multicore_mapping();
    }

    return true;
}

extern char _MPSTARTUP_START;
extern char _MPSTARTUP_END;
extern unsigned long mpcr3, mpstack, core_obj;
extern unsigned char mpready;
extern uint64_t *__template_pml4;

void infos::arch::x86::start_core(Core* core, LAPIC* lapic, PIT* pit) {
    uint8_t processor_id = core->get_processor_id();

    // Copy AP trampoline code onto start page
    uint8_t* start_page = (uint8_t*) pa_to_vpa(0);
    size_t mpstartup_size = (uint64_t)&_MPSTARTUP_END - (uint64_t)&_MPSTARTUP_START;
    memcpy(start_page, (const void *)(pa_to_vpa((uintptr_t) &_MPSTARTUP_START)), mpstartup_size);

    // Insert CR3 value
    uint64_t this_cr3 = (uint64_t) __template_pml4;
    size_t mpcr3_offset = (uint64_t)&mpcr3 - (uint64_t)&_MPSTARTUP_START;
    *((uint64_t *)pa_to_vpa(mpcr3_offset)) = this_cr3;		// HACK: ONLY WORKS IF BASE PFN=0

    // Insert RSP value
    size_t mprsp_offset = (uint64_t)&mpstack - (uint64_t)&_MPSTARTUP_START;
    PageDescriptor* pgd = sys.mm().pgalloc().alloc_pages(1);
    *((uint64_t *)pa_to_vpa(mprsp_offset)) = (sys.mm().pgalloc().pgd_to_kva(pgd) + 0x2000);

    // Insert pointer to core object
    size_t core_obj_offset = (uint64_t)&core_obj - (uint64_t)&_MPSTARTUP_START;
    *((uint64_t *)pa_to_vpa(core_obj_offset)) = (uint64_t) core;

    asm volatile("mfence" ::: "memory");

    // Get MPREADY flag
    size_t mpready_offset = (uint64_t)&mpready - (uint64_t)&_MPSTARTUP_START;
    volatile uint8_t *ready_flag = (volatile uint8_t *)pa_to_vpa(mpready_offset);
    *ready_flag = 0;

    // send init and wait 10ms
//    cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "sending init to core %u", processor_id);
    lapic->send_remote_init(processor_id, 0);
    pit->spin(10000000);

    // send sipi and wait 1ms
//    cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "sending sipi to core %u", processor_id);
    lapic->send_remote_sipi(processor_id, 0);
    pit->spin(1000000);

    if (!*ready_flag) {
        // send second sipi and wait 1s
        cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "resending sipi to core %u", processor_id);
        lapic->send_remote_sipi(processor_id,0);
        pit->spin(1000000000);
    }

    if (!*ready_flag) {
        cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "core %u error, skipping", processor_id);
        return;
    } else {
        core->set_state(Core::core_state::ONLINE);
        *ready_flag = 2;
        cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "core %u ready", processor_id);
    }

    cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "waiting for core %u to be ready", processor_id);

    // wait for core to finish setup before moving on
    while(!core->is_initialised()) {
        asm volatile ("nop");
    }

    cpu_log.messagef(infos::kernel::LogLevel::DEBUG, "core %u ready, moving on", processor_id);
}

/**
 * Constructs a new X86CPU object.
 */
X86CPU::X86CPU()
{

}
