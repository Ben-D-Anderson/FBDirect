#include <iostream>
#include <string>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <cstring>

class Device {
private:
    const fb_var_screeninfo m_screenInfo;
    const std::string m_devicePath;
    const unsigned int m_bytesOnLine, m_bytesOnScreen;
    const uint8_t m_bytesPerPixel;
    void* m_addr{nullptr};
    uint8_t* m_buffer;

    int createPixel(const uint8_t& red, const uint8_t& green, const uint8_t& blue, const uint8_t& transparency) const {
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
        return pixel;
    }

    static void reverseByte(uint8_t& b) {
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    }

public:
    Device(const fb_var_screeninfo screenInfo, std::string devicePath)
            : m_screenInfo(screenInfo), m_devicePath(std::move(devicePath)),
              m_bytesPerPixel(screenInfo.bits_per_pixel / 8),
              m_bytesOnLine(screenInfo.xres * (screenInfo.bits_per_pixel / 8)),
              m_bytesOnScreen(screenInfo.yres * screenInfo.xres * (screenInfo.bits_per_pixel / 8)) {
        m_buffer = static_cast<uint8_t*>(malloc(m_bytesOnScreen));
    }

    ~Device() {
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

    void setBufferPixel(const unsigned int x, const unsigned int y, const uint8_t& red, const uint8_t& green, const uint8_t& blue, const uint8_t& transparency) {
        int newPixel = createPixel(red, green, blue, transparency);
        uint8_t* newPixelPointer = static_cast<uint8_t*>(static_cast<void*>(&newPixel));
        uint8_t* pixel = m_buffer + m_bytesOnLine*y + x*m_bytesPerPixel;
        for (uint8_t i = 0; i < m_bytesPerPixel; i++) {
            *(pixel + i) = *(newPixelPointer + i);
        }
    }

    void renderBuffer() {
        memcpy(m_addr, m_buffer, m_bytesOnScreen);
    }

    void mapToMemory() {
        mapToMemory(nullptr);
    }

    void mapToMemory(void* targetAddress) {
        std::cout << "Mapping device " << m_devicePath << " to memory (requesting address " << targetAddress << ")\n";
        int fd = open(m_devicePath.c_str(), O_RDWR);
        void* addr {mmap(targetAddress, m_bytesOnScreen, PROT_WRITE, MAP_SHARED, fd, 0)};
        close(fd);
        m_addr = addr;
        std::cout << "Device " << m_devicePath << " was mapped to memory (" << m_addr << ")\n";
    }

    void unmapFromMemory() {
        if (m_addr == nullptr) {
            std::cout << "[!] Attempted to unmap nullptr from memory\n";
            return;
        }
        munmap(m_addr, m_bytesOnScreen);
        std::cout << "Device " << m_devicePath << " was unmapped from memory (" << m_addr << ")\n";
    }
};

int main() {
    const char* devicePath = "/dev/fb0";

    int fd = open(devicePath, O_RDWR);
    fb_var_screeninfo vinfo{};
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error: reading device information");
        return 1;
    }

    Device device{vinfo, devicePath};
    device.mapToMemory();
    for (int i = 0; i < 256; i++) {
        for (int y = 0; y < 1080; y++) {
            for (int x = 0; x < 1920; x++) {
                device.setBufferPixel(x, y, i, i, i, 255);
            }
        }
        device.renderBuffer();
    }

    return 0;
}
