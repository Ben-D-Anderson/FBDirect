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
    const unsigned int m_bytesOnLine;
    const uint8_t m_bytesPerPixel;
    void* m_addr{nullptr};

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

    static uint8_t decreaseBitLength(const uint8_t& sourceEightBit, const uint8_t& numberOfTargetBits) {
        uint8_t bits;
        if (numberOfTargetBits > 8) throw std::runtime_error("Illegal number of target bits - must be less than or equal to 8");
        else if (numberOfTargetBits < 8) bits = sourceEightBit >> (8 - numberOfTargetBits);
        else bits = sourceEightBit;
        return bits;
    }

public:
    Device(const fb_var_screeninfo screenInfo, std::string devicePath)
            : m_screenInfo(screenInfo), m_devicePath(std::move(devicePath)),
              m_bytesPerPixel(screenInfo.bits_per_pixel / 8), m_bytesOnLine(screenInfo.xres * (screenInfo.bits_per_pixel / 8)) {}

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

    void setPixel(const unsigned int x, const unsigned int y, const uint8_t& red, const uint8_t& green, const uint8_t& blue, const uint8_t& transparency) {
        int newPixel = createPixel(red, green, blue, transparency);
        uint8_t* pixel = (uint8_t*)m_addr + m_bytesOnLine*y + x*m_bytesPerPixel;
        memcpy(pixel, &newPixel, m_bytesPerPixel);
    }

//    int[] getPixel(const unsigned int x, const unsigned int y) {
//        if (!validPixelParams(x, y)) return -1;
//
//        uint8_t* pixel = (uint8_t*)m_addr + getWidth()*m_bytesPerPixel*y + x*m_bytesPerPixel;
//        int rawPixel = *(pixel);
//    }

    void mapToMemory() {
        mapToMemory(nullptr);
    }

    void mapToMemory(void* targetAddress) {
        std::cout << "Mapping device " << m_devicePath << " to memory (requesting address " << targetAddress << ")\n";
        int fd = open(m_devicePath.c_str(), O_RDWR);
        void* addr {mmap(targetAddress, getWidth() * getHeight() * m_screenInfo.bits_per_pixel / 8, PROT_WRITE, MAP_SHARED, fd, 0)};
        close(fd);
        m_addr = addr;
        std::cout << "Device " << m_devicePath << " was mapped to memory (" << m_addr << ")\n";
    }

    void unmapFromMemory() {
        if (m_addr == nullptr) {
            std::cout << "[!] Attempted to unmap nullptr from memory\n";
            return;
        }
        munmap(m_addr, getWidth() * getHeight() * m_screenInfo.bits_per_pixel / 8);
        std::cout << "Device " << m_devicePath << " was unmapped from memory (" << m_addr << ")\n";
    }

};

int main() {
    int fd = open("/dev/fb0", O_RDWR);
    fb_var_screeninfo vinfo{};
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error: reading device information");
        return 1;
    }

    Device device{vinfo, "/dev/fb0"};
    device.mapToMemory();
    for (int i = 0; i < 256; i++) {
        for (int y = 0; y < 1080; y++) {
            for (int x = 0; x < 1920; x++) {
                device.setPixel(x, y, i, i, i, 255);
            }
        }
    }

    return 0;
}
