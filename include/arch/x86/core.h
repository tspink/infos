/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/drivers/device.h>
#include <infos/kernel/irq.h>

namespace infos
{
    namespace arch
    {
        namespace x86
        {
            class Core {
            public:
                enum class core_state {
                    BOOTSTRAP,
                    ONLINE,
                    OFFLINE,
                    ERROR
                };

                Core();
                Core(uint8_t processor_id, uint8_t lapic_id, core_state state);
                uint8_t get_processor_id();
                uint8_t get_lapic_id();
                core_state get_state();

            private:
                uint8_t processor_id;
                uint8_t lapic_id;
                core_state state;
                // todo: maybe flags too?
            };
        }
    }
}


