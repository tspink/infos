#pragma once

#include <infos/drivers/device.h>

namespace infos
{
	namespace drivers
	{
		namespace video
		{
			struct BGRPixel {
				uint8_t blue;
				uint8_t green;
				uint8_t red;
				uint8_t pad;
				BGRPixel& operator=(const uint32_t a)
				{
					blue = a;
					green = a >> 8;
					red = a >> 16;
					pad = 0;
					return *this;
				}
			};

			class VideoDevice: public Device
			{
			public:
				static const DeviceClass VideoDeviceClass;

				const DeviceClass& device_class() const override { return VideoDeviceClass; }
				virtual phys_addr_t framebuffer() = 0;
				virtual size_t pitch() = 0;
				virtual size_t width() = 0;
				virtual size_t height() = 0;
				virtual void draw_text(uint16_t text_buffer[], uint32_t width, uint32_t height) = 0;
				size_t framebuffer_size_in_bytes();

				fs::File* open_as_file() override;
			};
		}
	}
}
