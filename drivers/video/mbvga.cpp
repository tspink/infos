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
}

const DeviceClass MBVGADevice::MBVGADeviceClass(MBVGADevice::MBVGADeviceClass, "video");
