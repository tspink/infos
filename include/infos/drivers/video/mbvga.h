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
				phys_addr_t framebuffer() override { return _framebuffer_address; }
				size_t pitch() override { return _framebuffer_pitch; }
				size_t width() override { return _framebuffer_width; }
				size_t height() override { return _framebuffer_height; }
				void draw_text(uint16_t text_buffer[], uint32_t width, uint32_t height) override;

			private:
				phys_addr_t _framebuffer_address;
				uint32_t _framebuffer_pitch, _framebuffer_width, _framebuffer_height;
				const uint32_t _font_width, _font_height;
				uint8_t _byte_bit_map[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
				void clear_screen();
				void draw_char(BGRPixel write_base[], char character, int flags=0);
			};
		}
	}
}

