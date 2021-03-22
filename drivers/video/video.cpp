#include <infos/drivers/video/video.h>
#include <infos/fs/file.h>

using namespace infos::mm;
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

	// Use memory mapped IO for framebuffer graphics
	bool mmap(VMA& vma, virt_addr_t addr, size_t len, off_t offset) override {
		size_t bytes_to_mmap = __min(len, _device.framebuffer_size_in_bytes());
		size_t nr_pages      = __align_up_page(bytes_to_mmap) >> __page_bits;
		auto frame_base = _device.framebuffer();

		// Add mappings
		for (size_t page = 0; page < nr_pages; page++) {
            auto addr_offset = page << __page_bits;
			vma.insert_mapping(addr + addr_offset, frame_base + addr_offset, MappingFlags::Present | MappingFlags::User | MappingFlags::Writable);
		}
		return true;
	}

private:
    VideoDevice& _device;
};

size_t VideoDevice::framebuffer_size_in_bytes()
{
	return pitch() * height();
}

File *VideoDevice::open_as_file()
{
	return new VideoDeviceFile(*this);
}
