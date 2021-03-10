#pragma once

#include <infos/drivers/device.h>

namespace infos
{
	namespace drivers
	{
		namespace video
		{
			class VideoDevice: public Device
			{
			public:
				static const DeviceClass VideoDeviceClass;

				const DeviceClass& device_class() const override { return VideoDeviceClass; }

				fs::File* open_as_file() override;
			};
		}
	}
}
