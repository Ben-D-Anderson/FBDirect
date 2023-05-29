#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <unistd.h>

#include "device.h"

Device::Device(const ScreenInfo screenInfo, std::string_view devicePath)
	: m_screenInfo(screenInfo),
	m_devicePath(devicePath),
	m_bytesPerPixel(screenInfo.var.bits_per_pixel / 8),
	m_bytesOnLine(screenInfo.fix.line_length),
	m_pixelBytesOnLine(screenInfo.var.xres * (screenInfo.var.bits_per_pixel / 8)),
	m_bytesOnScreen(screenInfo.var.yres * m_bytesOnLine) {
		m_buffer = static_cast<uint8_t*>(malloc(m_bytesOnScreen));
	}

Device::~Device() {
	unmapFromMemory();
	free(m_buffer);
}

//creates a new pixel on the stack every time this method is called
void Device::setBufferPixel(const unsigned int& x, const unsigned int& y, const uint8_t& red, const uint8_t& green, const uint8_t& blue, const uint8_t& transparency) {
	Pixel newPixel = createPixel(m_screenInfo, red, green, blue, transparency);
	setBufferPixel(x, y, newPixel);
}

//can re-use created pixels
void Device::setBufferPixel(const unsigned int& x, const unsigned int& y, Pixel& pixel) {
	uint8_t* newPixelPointer = static_cast<uint8_t*>(static_cast<void*>(&pixel.data));
	uint8_t* currentPixel = m_buffer + m_bytesOnLine*y + x*m_bytesPerPixel;
	for (uint8_t i = 0; i < m_bytesPerPixel; i++) {
		*(currentPixel + i) = *(newPixelPointer + i);
	}
}

Pixel Device::getBufferPixel(const unsigned int& x, const unsigned int& y) {
	int data = 0;
	uint8_t* currentPixel = m_buffer + m_bytesOnLine*y + x*m_bytesPerPixel;
	for (uint8_t i = 0; i < m_bytesPerPixel; i++) {
		data |= *(currentPixel + i) << (i * m_bytesPerPixel);
	}
	return parsePixel(m_screenInfo, data);
}

void Device::mapToMemory() {
	std::cout << "Mapping device " << m_devicePath << " to memory\n";
	int fd = open(m_devicePath.data(), O_RDWR);
	void* addr {mmap(nullptr, m_bytesOnScreen, PROT_WRITE, MAP_SHARED, fd, 0)};
	close(fd);
	m_addr = addr;
	std::cout << "Device " << m_devicePath << " was mapped to memory (" << m_addr << ")\n";
}

void Device::unmapFromMemory() {
	if (m_addr == nullptr) {
		return;
	}
	munmap(m_addr, m_bytesOnScreen);
	std::cout << "Device " << m_devicePath << " was unmapped from memory (" << m_addr << ")\n";
	m_addr = nullptr;
}
