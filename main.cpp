#include <iostream>
#include <string>
#include <utility>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <chrono>

#include "framebuffer.h"
#include "pixel.h"

class Device {
	private:
		const ScreenInfo m_screenInfo;
		const std::string m_devicePath;
		const unsigned int m_bytesPerPixel;
		const unsigned int m_bytesOnLine, m_pixelBytesOnLine, m_bytesOnScreen;
		void* m_addr{nullptr};
		uint8_t* m_buffer;

	public:
		const void blankBuffer() {
			memset(m_buffer, 0, m_bytesOnScreen);
		}

		Device(const ScreenInfo screenInfo, std::string devicePath)
			: m_screenInfo(screenInfo),
			m_devicePath(devicePath),
			m_bytesPerPixel(screenInfo.var.bits_per_pixel / 8),
			m_bytesOnLine(screenInfo.fix.line_length),
			m_pixelBytesOnLine(screenInfo.var.xres * (screenInfo.var.bits_per_pixel / 8)),
			m_bytesOnScreen(screenInfo.var.yres * m_bytesOnLine) {
				m_buffer = static_cast<uint8_t*>(malloc(m_bytesOnScreen));
			}

		~Device() {
			unmapFromMemory();
			free(m_buffer);
		}

		Device(const Device& other) = delete;

		std::string getDevicePath() {
			return m_devicePath;
		}

		unsigned int getWidth() const {
			return m_screenInfo.var.xres;
		}

		unsigned int getHeight() const {
			return m_screenInfo.var.yres;
		}

		void* getMappedAddr() {
			return m_addr;
		}

		//creates a new pixel on the stack every time this method is called
		void setBufferPixel(const unsigned int& x, const unsigned int& y, const uint8_t& red, const uint8_t& green, const uint8_t& blue, const uint8_t& transparency) {
			Pixel newPixel = createPixel(m_screenInfo, red, green, blue, transparency);
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
			return parsePixel(m_screenInfo, data);
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
	ScreenInfo info{getScreenInfo(fd)};
	activateScreen(fd, info);

	Device device{info, devicePath};
	device.mapToMemory();

	auto begin = std::chrono::high_resolution_clock::now();
	for (unsigned int i = 0; i < 256; i++) {
		Pixel pixel = createPixel(info, 255-i, i, 255-i, 255);
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

	device.blankBuffer();
	device.renderBuffer();
	
	device.unmapFromMemory();
	close(fd);
	showConsoleCursor();

	return 0;
}
