#include <infos/drivers/irq/core.h>
#include <infos/kernel/kernel.h>
#include <arch/x86/msr.h>
#include <arch/x86/cpuid.h>

using namespace infos::arch::x86;
using namespace infos::drivers;
using namespace infos::drivers::irq;
using namespace infos::kernel;

const DeviceClass infos::drivers::irq::Core::CoreDeviceClass(RootDeviceClass, "core");
Core* Core::cores[32] = {};

Core::Core(uint8_t processor_id, uint8_t lapic_id, core_state state) :
        processor_id(processor_id), lapic_id(lapic_id), state(state), scheduler(sys) {}

bool Core::init(kernel::DeviceManager& dm) { return true; }

uint8_t Core::get_processor_id() { return processor_id; }
uint8_t Core::get_lapic_id() { return lapic_id; }
Core::core_state Core::get_state() { return state; }

Scheduler& Core::get_scheduler() { return scheduler; }
bool Core::is_initialised() { return initialised; }
void Core::set_initialised(bool initialised) { this->initialised = initialised; }

void Core::set_state(Core::core_state) { Core::state = state; }
void Core::add_core(Core* new_core) { Core::cores[new_core->get_lapic_id()] = new_core; }

Core* Core::get_current_core() {
    uint8_t apic_id = (*(uint32_t *)(pa_to_vpa((__rdmsr(MSR_APIC_BASE) & ~0xfff) + 0x20))) >> 24;
    return cores[apic_id];
}

Core** Core::get_cores() { return cores; }

bool Core::sched_init() { return scheduler.init(); }

/**
 * Registers the LAPIC of the currently executing core with the device manager.
 * @return Returns TRUE if the LAPIC was successfully registered with the device manager, or FALSE otherwise.
 */
bool Core::lapic_init() {
    // LAPIC base is the same for every LAPIC in the system, you can
    // only access the registers of the LAPIC of the currently executing core
    uint64_t lapic_base = __rdmsr(MSR_APIC_BASE) & ~0xfff;
    if (!lapic_base) {
        syslog.messagef(LogLevel::ERROR, "Invalid LAPIC base address %x", lapic_base);
        return false;
    }

    // Create the core's LAPIC object and register with the device manager
    lapic = new LAPIC(pa_to_vpa(lapic_base));

    return sys.device_manager().register_device(*lapic);
}

bool Core::timer_init() {
    // Read the CPU feature set.
    CPUIDFeatures::CPUIDFeatures features = cpuid_get_features();

    // The timer requires the APIC, so check that it is present.
    if (!(features.rdx & CPUIDFeatures::APIC)) {
        syslog.message(LogLevel::ERROR, "APIC not present");
        return false;
    }

    // Create and register the LAPIC timer. When calibrating, the LAPIC uses the PIT as a
    // reference timer. The PIT is registered in cpu_init and presence is checked during
    // LAPIC init before the LAPIC begins to calibrate.
    infos::drivers::timer::LAPICTimer *lapic_timer = new infos::drivers::timer::LAPICTimer();
    lapic_timer->set_lapic_ptr(this->lapic);

    if (!sys.device_manager().register_device(*lapic_timer)) {
        syslog.message(LogLevel::ERROR, "LAPIC Timer failed to initialise");
        return false;
    }

    // Set the timer to be periodic, with a period of 10ms, and start
    // the timer.
    lapic_timer->init_periodic((lapic_timer->frequency() >> 4) / 100);
    lapic_timer->start();

    return true;
}