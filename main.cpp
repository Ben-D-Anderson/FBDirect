#include <iostream>
#include <string>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <cstring>
#include <chrono>

class Device {
private:
    const fb_var_screeninfo m_screenInfo;
    const std::string m_devicePath;
    const uint8_t m_bytesPerPixel;
    const unsigned int m_bytesOnLine, m_bytesOnScreen;
    void* m_addr{nullptr};
    uint8_t* m_buffer;

    static void reverseByte(uint8_t& b) {
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    }
	
	static unsigned char createBitMask(__u32 length) {
		unsigned char byte = 0;
		for (unsigned int i = 0; i < length; i++) {
        	byte |= (1 << i);
    	}
		return byte;
	}
public:
	struct Pixel {
		int data;
		uint8_t red, green, blue, transparency;
		Pixel(const uint8_t& _red, const uint8_t& _green, const uint8_t& _blue, const uint8_t& _transparency) {
			red = _red;
			green = _green;
			blue = _blue;
			transparency = _transparency;
		}
	};

	Pixel parsePixel(int data) {
		uint8_t redBits = data >> m_screenInfo.red.offset;
		redBits = redBits & createBitMask(m_screenInfo.red.length);
		if (m_screenInfo.red.msb_right != 0) reverseByte(redBits);
		uint8_t greenBits = data >> m_screenInfo.green.offset;
		greenBits = greenBits & createBitMask(m_screenInfo.green.length);
		if (m_screenInfo.green.msb_right != 0) reverseByte(greenBits);
		uint8_t blueBits = data >> m_screenInfo.blue.offset;
		blueBits = blueBits & createBitMask(m_screenInfo.blue.length);
		if (m_screenInfo.blue.msb_right != 0) reverseByte(blueBits);
		uint8_t transparentBits = data >> m_screenInfo.transp.offset;
		transparentBits = transparentBits & createBitMask(m_screenInfo.transp.length);
		if (m_screenInfo.transp.msb_right != 0) reverseByte(transparentBits);

		redBits = redBits << (8 - m_screenInfo.red.length);
		greenBits = greenBits << (8 - m_screenInfo.green.length);
		blueBits = blueBits << (8 - m_screenInfo.blue.length);
		transparentBits = transparentBits << (8 - m_screenInfo.transp.length);
		
		Pixel pixel{redBits, greenBits, blueBits, transparentBits};
		pixel.data = data;
		return pixel;
	}

    Pixel createPixel(const uint8_t& red, const uint8_t& green, const uint8_t& blue, const uint8_t& transparency) const {
        uint8_t redBits = red >> (8 - m_screenInfo.red.length);
        if (m_screenInfo.red.msb_right != 0) reverseByte(redBits);
        uint8_t greenBits = green >> (8 - m_screenInfo.green.length);
        if (m_screenInfo.green.msb_right != 0) reverseByte(greenBits);
        uint8_t blueBits = blue >> (8 - m_screenInfo.blue.length);
        if (m_screenInfo.blue.msb_right != 0) reverseByte(blueBits);
        uint8_t transparentBits = transparency >> (8 - m_screenInfo.transp.length);
        if (m_screenInfo.transp.msb_right != 0) reverseByte(transparentBits);
		
        int pixel = 0;
        if (m_screenInfo.red.length > 0) pixel += redBits << m_screenInfo.red.offset;
        if (m_screenInfo.green.length > 0) pixel += greenBits << m_screenInfo.green.offset;
        if (m_screenInfo.blue.length > 0) pixel += blueBits << m_screenInfo.blue.offset;
        if (m_screenInfo.transp.length > 0) pixel += transparentBits << m_screenInfo.transp.offset;
        
		Pixel pixelStruct{red, green, blue, transparency};
		pixelStruct.data = pixel;
		return pixelStruct;
    }

	static const fb_var_screeninfo getScreenInfo(int* fileDescriptor) {
    	fb_var_screeninfo vinfo{};
		if (ioctl(*fileDescriptor, FBIOGET_VSCREENINFO, &vinfo) == -1) {
			perror("Error reading device information");
    	}
		return vinfo;
	}
	
	//returns boolean denoting success of activation
	static const bool activateScreen(int* fileDescriptor, fb_var_screeninfo& screenInfo) {
		screenInfo.activate |= FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
		if(ioctl(*fileDescriptor, FBIOPUT_VSCREENINFO, &screenInfo) == -1) {
  			perror("Error activing frame buffer");
			return false;
		}
		return true;
	}

    Device(const fb_var_screeninfo screenInfo, std::string devicePath)
            : m_screenInfo(screenInfo), m_devicePath(std::move(devicePath)),
              m_bytesPerPixel(screenInfo.bits_per_pixel / 8),
              m_bytesOnLine(screenInfo.xres * (screenInfo.bits_per_pixel / 8)),
              m_bytesOnScreen(screenInfo.yres * screenInfo.xres * (screenInfo.bits_per_pixel / 8)) {
        m_buffer = static_cast<uint8_t*>(malloc(m_bytesOnScreen));
    }

    ~Device() {
		unmapFromMemory();
        free(m_buffer);
    }

    std::string getDevicePath() {
        return m_devicePath;
    }

    unsigned int getWidth() const {
        return m_screenInfo.xres;
    }

    unsigned int getHeight() const {
        return m_screenInfo.yres;
    }

    void* getMappedAddr() {
        return m_addr;
    }

	//creates a new pixel on the stack every time this method is called
    void setBufferPixel(const unsigned int& x, const unsigned int& y, const uint8_t& red, const uint8_t& green, const uint8_t& blue, const uint8_t& transparency) {
        Pixel newPixel = createPixel(red, green, blue, transparency);
    	setBufferPixel(x, y, newPixel);
	}

	//can re-use created pixels
	void setBufferPixel(const unsigned int& x, const unsigned int& y, Pixel& pixel) {
		uint8_t* newPixelPointer = static_cast<uint8_t*>(static_cast<void*>(&pixel.data));
        uint8_t* currentPixel = m_buffer + m_bytesOnLine*y + x*m_bytesPerPixel;
        for (uint8_t i = 0; i < m_bytesPerPixel; i++) {
            *(currentPixel + i) = *(newPixelPointer + i);
        }
	}

	Pixel getBufferPixel(const unsigned int& x, const unsigned int& y) {
		int data = 0;
		uint8_t* currentPixel = m_buffer + m_bytesOnLine*y + x*m_bytesPerPixel;
		for (uint8_t i = 0; i < m_bytesPerPixel; i++) {
			data |= *(currentPixel + i) << (i * m_bytesPerPixel);
		}
		return parsePixel(data);
	}

    void renderBuffer() {
		memcpy(m_addr, m_buffer, m_bytesOnScreen);
    }

    void mapToMemory() {
        std::cout << "Mapping device " << m_devicePath << " to memory\n";
        int fd = open(m_devicePath.c_str(), O_RDWR);
        void* addr {mmap(nullptr, m_bytesOnScreen, PROT_WRITE, MAP_SHARED, fd, 0)};
        close(fd);
        m_addr = addr;
        std::cout << "Device " << m_devicePath << " was mapped to memory (" << m_addr << ")\n";
    }

    void unmapFromMemory() {
        if (m_addr == nullptr) {
            return;
        }
        munmap(m_addr, m_bytesOnScreen);
        std::cout << "Device " << m_devicePath << " was unmapped from memory (" << m_addr << ")\n";
    	m_addr = nullptr;
	}
};

void hideConsoleCursor() {
	fputs("\e[?25l", stdout);
}

void showConsoleCursor() {
	fputs("\e[?25h", stdout);
}

int main() {
	hideConsoleCursor();
	
	const char* devicePath = "/dev/fb0";
	int fd = open(devicePath, O_RDWR);

    fb_var_screeninfo vinfo{Device::getScreenInfo(&fd)};
	if (vinfo.xres == 0 || vinfo.yres == 0)
		return 1;
	Device::activateScreen(&fd, vinfo);

    Device device{vinfo, devicePath};
	device.mapToMemory();
	
	auto begin = std::chrono::high_resolution_clock::now();
    for (unsigned int i = 0; i < 256; i++) {
		Device::Pixel pixel = device.createPixel(255-i, i, 255-i, 255);
		for (unsigned int y = 0; y < device.getHeight(); y++) {
            for (unsigned int x = 0; x < device.getWidth(); x++) {
                device.setBufferPixel(x, y, pixel);
            }
        }
		device.renderBuffer();
	}
	auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    printf("Time measured: %.3f seconds.\n", elapsed.count() * 1e-9);
	
	device.unmapFromMemory();

	close(fd);
	
	showConsoleCursor();

    return 0;
}
