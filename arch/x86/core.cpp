#include "core.h"

using namespace infos::drivers;
using namespace infos::arch::x86;

Core::Core(uint8_t processor_id, uint8_t lapic_id, core_state state) :
    processor_id(processor_id), lapic_id(lapic_id), state(state) {}
uint8_t Core::get_processor_id() { return processor_id; }
uint8_t Core::get_lapic_id() { return lapic_id; }
Core::core_state Core::get_state() { return state; }