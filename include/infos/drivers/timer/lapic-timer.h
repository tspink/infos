/* SPDX-License-Identifier: MIT */

/*
 * include/drivers/timer/lapic.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/drivers/timer/timer.h>

namespace infos {
	namespace kernel {
		class IRQ;
	}
	
	namespace drivers {
		namespace irq {
			class LAPIC;
		}
		
		namespace timer {
			class LAPICTimer : public Timer {
			public:
				static const DeviceClass LAPICTimerDeviceClass;

				LAPICTimer();

                void set_lapic_ptr(drivers::irq::LAPIC * lapic);

                const DeviceClass& device_class() const override {
					return LAPICTimerDeviceClass;
				}
				
				bool init(kernel::DeviceManager& dm) override;

				void init_oneshot(uint64_t period) override;
				void init_periodic(uint64_t period) override;

				void start() override;
				void stop() override;
				void reset() override;
				
				uint64_t count() const override;
				bool expired() const override;

				uint64_t frequency() const override { return _frequency; }

			private:
				uint64_t _frequency;

				kernel::IRQ *_irq;
				drivers::irq::LAPIC *_lapic;

				static void lapic_timer_irq_handler(const kernel::IRQ *irq, void *priv);
				bool calibrate();
			};
		}
	}
}
