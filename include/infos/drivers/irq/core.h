#pragma once

#include <infos/drivers/device.h>
#include <infos/kernel/irq.h>
#include <infos/kernel/sched.h>
#include <infos/drivers/irq/lapic.h>

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
                kernel::Scheduler* get_sched_ptr();

                void set_state(core_state state);
                void set_lapic_ptr(LAPIC *lapic);
                void set_sched_ptr(kernel::Scheduler* sched_ptr);

                //todo: have a private runqueue!

            private:
                uint8_t processor_id;
                uint8_t lapic_id;
                core_state state;
                LAPIC *lapic_ptr;
                kernel::Scheduler *sched_ptr;
            };
        }
    }
}