#include <infos/drivers/irq/core.h>

using namespace infos::drivers;
using namespace infos::drivers::irq;

const DeviceClass infos::drivers::irq::Core::CoreDeviceClass(RootDeviceClass, "core");

Core::Core(uint8_t processor_id, uint8_t lapic_id, core_state state) :
        processor_id(processor_id), lapic_id(lapic_id), state(state) {}

bool Core::init(kernel::DeviceManager& dm) { return true; }

uint8_t Core::get_processor_id() { return processor_id; }
uint8_t Core::get_lapic_id() { return lapic_id; }
Core::core_state Core::get_state() { return state; }

void Core::set_state(Core::core_state) { Core::state = state; }
void Core::set_lapic_ptr(LAPIC *lapic_ptr) { Core::lapic_ptr = lapic_ptr; }