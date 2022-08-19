#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

class Device {
private:
    const int m_width, m_height, m_bytesPerPixel;
    std::string m_devicePath;
    void* m_addr{nullptr};

public:
    Device(int width, int height, int bitsPerPixel, int deviceNumber)
            : m_width(width), m_height(height), m_bytesPerPixel(bitsPerPixel/8) {
        m_devicePath = std::string("/dev/fb") + std::to_string(deviceNumber);
    }

    std::string getDevicePath() {
        return m_devicePath;
    }

    unsigned int getWidth() const {
        return m_width;
    }

    unsigned int getHeight() const {
        return m_height;
    }

    void* getMappedAddr() {
        return m_addr;
    }

    void setPixel(unsigned int x, unsigned int y, unsigned char a, unsigned char b) {
        if (m_addr == nullptr) {
            std::cout << "[!] Attempted to setPixel on nullptr\n";
            return;
        }
        if (x >= m_width) {
            std::cout << "x parameter (" << x << ") in Device#setPixel is outside acceptable range\n";
            return;
        }
        if (y >= m_height) {
            std::cout << "y parameter (" << y << ") in Device#setPixel is outside acceptable range\n";
            return;
        }
        unsigned char* pixel = (unsigned char*)m_addr + m_width*m_bytesPerPixel*y + x*m_bytesPerPixel;
        *(pixel) = a;
        *(pixel+1) = b;
    }

    int getPixel(unsigned int x, unsigned int y) {
        if (m_addr == nullptr) {
            std::cout << "[!] Attempted to getPixel on nullptr\n";
            return -1;
        }
        if (x >= m_width) {
            std::cout << "x parameter (" << x << ") in Device#setPixel is outside acceptable range\n";
            return -1;
        }
        if (y >= m_height) {
            std::cout << "y parameter (" << y << ") in Device#setPixel is outside acceptable range\n";
            return -1;
        }
        unsigned char* pixel = (unsigned char*)m_addr + m_width*m_bytesPerPixel*y + x*m_bytesPerPixel;
        return *(pixel);
    }

    void mapToMemory() {
        std::cout << "Mapping device " << m_devicePath << " to memory\n";
        int fd = open(m_devicePath.c_str(), O_RDWR);
        void* addr {mmap(nullptr, getWidth() * getHeight() * m_bytesPerPixel, PROT_WRITE, MAP_SHARED, fd, 0)};
        close(fd);
        m_addr = addr;
        std::cout << "Device " << m_devicePath << " was mapped to memory (" << m_addr << ")\n";
    }

    void mapToMemory(void* targetAddress) {
        std::cout << "Mapping device " << m_devicePath << " to memory (requesting address " << targetAddress << ")\n";
        int fd = open(m_devicePath.c_str(), O_RDWR);
        void* addr {mmap(targetAddress, getWidth() * getHeight() * m_bytesPerPixel, PROT_WRITE, MAP_SHARED, fd, 0)};
        close(fd);
        m_addr = addr;
        std::cout << "Device " << m_devicePath << " was mapped to memory (" << m_addr << ")\n";
    }

    void unmapFromMemory() {
        if (m_addr == nullptr) {
            std::cout << "[!] Attempted to unmap nullptr from memory\n";
            return;
        }
        munmap(m_addr, getWidth() * getHeight() * m_bytesPerPixel);
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
    std::cout << "red.offset: " << vinfo.red.offset << "\n";
    std::cout << "red.length: " << vinfo.red.length << "\n";
    std::cout << "red.msb_right: " << vinfo.red.msb_right << "\n";
    std::cout << "blue.offset: " << vinfo.blue.offset << "\n";
    std::cout << "blue.length: " << vinfo.blue.length << "\n";
    std::cout << "blue.msb_right: " << vinfo.blue.msb_right << "\n";
    std::cout << "green.offset: " << vinfo.green.offset << "\n";
    std::cout << "green.length: " << vinfo.green.length << "\n";
    std::cout << "green.msb_right: " << vinfo.green.msb_right << "\n";

    Device device{static_cast<int>(vinfo.xres_virtual),
                  static_cast<int>(vinfo.yres_virtual),
                  static_cast<int>(vinfo.bits_per_pixel),
                  0};
    device.mapToMemory();
    for (int y = 0; y < 1080; y++) {
        for (int x = 0; x < 1920; x++) {
            //tv says: 5 bits blue - 6 bits green - 5 bits red
            //REVERSE EACH BYTE THEN COMBINE THEM - BECAUSE OF msb_right
            device.setPixel(x, y, 0b00000000, 0b11111000);
        }
    }

    return 0;
}
