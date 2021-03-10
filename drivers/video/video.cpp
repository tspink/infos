#include <infos/drivers/video/video.h>
#include <infos/fs/file.h>

using namespace infos::fs;
using namespace infos::drivers;
using namespace infos::drivers::video;

const DeviceClass VideoDevice::VideoDeviceClass(Device::RootDeviceClass, "video");


class VideoDeviceFile: public File
{
public:
    VideoDeviceFile(VideoDevice& device) : _device(device) {}

    void close() override {}

    int pread(void *ubuffer, size_t size, off_t off) override { return 0; }

    int read(void* ubuffer, size_t size) override { return 0; }

    void seek(off_t offset, SeekType type) override {}

    int write(const void* buffer, size_t size) override { return 0; }

private:
    VideoDevice& _device;
};

File *VideoDevice::open_as_file()
{
	return new VideoDeviceFile(*this);
}
