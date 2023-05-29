#ifndef PIXEL_H
#define PIXEL_H

#include <cstdint>
#include "framebuffer.h"

struct Pixel {
	int data;
	uint8_t red, green, blue, transparency;
	Pixel(const uint8_t& _red, const uint8_t& _green, const uint8_t& _blue, const uint8_t& _transparency);
};

Pixel parsePixel(const ScreenInfo& screeninfo, const int& data);
Pixel createPixel(const ScreenInfo& screeninfo, const uint8_t& _red, const uint8_t& _green, const uint8_t& _blue, const uint8_t& _transparency);

unsigned char createBitMask(__u32 length);
void reverseByte(uint8_t& b);

#endif
