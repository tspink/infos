/**
 * MBVGA Device implements graphics with the framebuffer provided by multiboot
 */

#include <infos/drivers/video/mbvga.h>
#include <infos/kernel/log.h>

using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::drivers::video;

ComponentLog mbvga_log(syslog, "mbvga");

MBVGADevice::MBVGADevice(phys_addr_t addr, uint32_t pitch, uint32_t width, uint32_t height)
    : _framebuffer_address(addr)
    , _framebuffer_pitch(pitch)
    , _framebuffer_width(width)
    , _framebuffer_height(height)
{
    // Obtain multiboot VGA framebuffer pointer
    mbvga_log.messagef(LogLevel::DEBUG, "MBVGA framebuffer address: %p", addr);
    mbvga_log.messagef(LogLevel::DEBUG, "MBVGA framebuffer pitch: %u", pitch);
    mbvga_log.messagef(LogLevel::DEBUG, "MBVGA framebuffer width: %u", width);
    mbvga_log.messagef(LogLevel::DEBUG, "MBVGA framebuffer height: %u", height);
    uint32_t bytes_per_pixel = _framebuffer_pitch / _framebuffer_width;
    while (true) {
        for (uint32_t channel = 0; channel < 3; channel++) {
            for (uint32_t val = 0; val < 0xff; val++) {
                uint32_t pix_val = val << (channel * 8);
                for (uint32_t pixel = 0; pixel < pitch * height; pixel += bytes_per_pixel) {
                    *(uint32_t *)(pa_to_vpa(_framebuffer_address) + pixel) = pix_val;
                }
            }
        }
    }
    util::memset((void *)pa_to_vpa(_framebuffer_address), 0xff, pitch * height);
}

const DeviceClass MBVGADevice::MBVGADeviceClass(MBVGADevice::MBVGADeviceClass, "video");
