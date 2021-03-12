/**
 * MBVGA Device implements graphics with the framebuffer provided by multiboot
 */

#include <infos/kernel/log.h>
#include <infos/drivers/video/mbvga.h>
#include <infos/drivers/video/ter-i16n.h>

using namespace infos::util;
using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::drivers::video;

ComponentLog mbvga_log(syslog, "mbvga");

MBVGADevice::MBVGADevice(phys_addr_t addr, uint32_t pitch, uint32_t width, uint32_t height)
	: _framebuffer_address(addr)
	, _framebuffer_pitch(pitch)
	, _framebuffer_width(width)
	, _framebuffer_height(height)
	, _font_width(8)
	, _font_height(16)
{
	// Obtain multiboot VGA framebuffer pointer
	mbvga_log.messagef(LogLevel::DEBUG, "MBVGADevice framebuffer address=%p, pitch=%lu, width=%lu, height=%lu", addr, pitch, width, height);

	// Ensure framebuffer is page aligned
	assert((addr & (__page_size - 1)) == 0);
}

void MBVGADevice::draw_text(uint16_t text_buffer[], uint32_t width, uint32_t height)
{
	// Clear framebuffer
	clear_screen();

	for (uint32_t row = 0; row < height; row++) {
		for (uint32_t col = 0; col < width; col++) {
			auto text_char = text_buffer[row * width + col];
			// TODO: Handle text attributes colour etc.
			if ((text_char & 0xff) == 0) {
				continue;
			}
			virt_addr_t fb_write_base = pa_to_vpa(_framebuffer_address) + _framebuffer_pitch * row * _font_height + col * _font_width * sizeof(BGRPixel);
			draw_char(reinterpret_cast<BGRPixel *>(fb_write_base), text_char);
		}
	}
}

void MBVGADevice::draw_char(BGRPixel write_base[], char text_char, int flags)
{
	uint8_t *read_ptr = (uint8_t *) font_16_raw + ((uintptr_t)text_char * _font_height);
	// Draw a single character
	for (size_t y = 0; y < _font_height; y++) {
		auto font_char = read_ptr[y];
		for (size_t x = 0; x < _font_width; x++) {
			write_base[x + y * _framebuffer_width] = (font_char & _byte_bit_map[x]) ? -1 : 0;
		}
	}
}

void MBVGADevice::clear_screen()
{
	bzero((void *)pa_to_vpa(_framebuffer_address), _framebuffer_pitch * _framebuffer_height);
}

const DeviceClass MBVGADevice::MBVGADeviceClass(VideoDevice::VideoDeviceClass, "video");
