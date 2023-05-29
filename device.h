#ifndef DEVICE_H
#define DEVICE_H

#include <cstring>
#include <string_view>

#include "pixel.h"
#include "framebuffer.h"

class Device {
	private:
		const ScreenInfo m_screenInfo;
		const std::string_view m_devicePath;
		const unsigned int m_bytesPerPixel;
		const unsigned int m_bytesOnLine, m_pixelBytesOnLine, m_bytesOnScreen;
		void* m_addr{nullptr};
		uint8_t* m_buffer;
	public:
		Device(const ScreenInfo screenInfo, std::string_view devicePath);
		~Device();
		Device(const Device& other) = delete;

		void setBufferPixel(const unsigned int& x, const unsigned int& y, const uint8_t& red, const uint8_t& green, const uint8_t& blue, const uint8_t& transparency);
		void setBufferPixel(const unsigned int& x, const unsigned int& y, Pixel& pixel);
		Pixel getBufferPixel(const unsigned int& x, const unsigned int& y);
		void mapToMemory();
		void unmapFromMemory();
		
		inline void blankBuffer() {
			memset(m_buffer, 0, m_bytesOnScreen);
		}

		inline void renderBuffer() {
			memcpy(m_addr, m_buffer, m_bytesOnScreen);
		}

		inline std::string_view getDevicePath() const {
			return m_devicePath;
		}

		inline unsigned int getWidth() const {
			return m_screenInfo.var.xres;
		}

		inline unsigned int getHeight() const {
			return m_screenInfo.var.yres;
		}

		inline void* getMappedAddr() const {
			return m_addr;
		}
};

#endif
