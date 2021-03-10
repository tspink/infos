#pragma once

#include <infos/drivers/video/video.h>

namespace infos
{
	namespace drivers
	{
		namespace video
		{
			class MBVGADevice: public VideoDevice
			{
			public:
				static const DeviceClass MBVGADeviceClass;

                MBVGADevice(phys_addr_t addr, uint32_t pitch, uint32_t width, uint32_t height);
				~MBVGADevice() {};

				const DeviceClass& device_class() const override { return MBVGADeviceClass; }

            private:
                phys_addr_t _framebuffer_address;
                uint32_t _framebuffer_pitch, _framebuffer_width, _framebuffer_height;
                uint32_t framebuffer_size_in_bytes() const { return _framebuffer_pitch * _framebuffer_height; }
			};
		}
	}
}

