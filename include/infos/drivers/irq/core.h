#pragma once

#include <infos/drivers/device.h>
#include <infos/kernel/irq.h>
#include <infos/kernel/sched.h>
#include <infos/drivers/irq/lapic.h>
#include <infos/util/map.h>

namespace infos {
    namespace drivers {
        namespace irq {
            class Core : public Device {
            public:
                enum class core_state {
                    BOOTSTRAP,
                    ONLINE,
                    OFFLINE,
                    ERROR
                };

                static const DeviceClass CoreDeviceClass;
                const DeviceClass& device_class() const override { return CoreDeviceClass; }

                bool init(kernel::DeviceManager& dm) override;

                Core(uint8_t processor_id, uint8_t lapic_id, core_state state);
                uint8_t get_processor_id();
                uint8_t get_lapic_id();
                core_state get_state();
                kernel::Scheduler& get_scheduler();
                bool is_initialised();
                void set_initialised(bool initialised);

                bool sched_init();
                bool lapic_init();
                bool timer_init();

                void set_state(core_state state);

                static void add_core(Core* new_core);
                static Core* get_current_core();
                static Core** get_cores();

            private:
                uint8_t processor_id;
                uint8_t lapic_id;
                core_state state;
                LAPIC *lapic;
                kernel::Scheduler scheduler;
                volatile bool initialised;
                static Core* cores[32];
            };
        }
    }
}