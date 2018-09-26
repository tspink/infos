/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/drivers/timer/timer.h>

namespace infos
{
	namespace drivers
	{
		namespace timer
		{
			class PIT : public Timer
			{
			public:
				static const DeviceClass PITDeviceClass;
				const DeviceClass& device_class() const override { return PITDeviceClass; }
				
				PIT();
								
				void init_oneshot(uint64_t period) override;
				void init_periodic(uint64_t period) override;

				void start() override;
				void stop() override;
				void reset() override;
				
				bool expired() const override;

				uint64_t count() const override;
				uint64_t frequency() const override { return 1193180; }
			};
		}
	}
}
