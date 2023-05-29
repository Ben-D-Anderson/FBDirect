#include "pixel.h"

Pixel::Pixel(const uint8_t& _red, const uint8_t& _green, const uint8_t& _blue, const uint8_t& _transparency) {
	red = _red;
	green = _green;
	blue = _blue;
	transparency = _transparency;
}

Pixel parsePixel(const ScreenInfo& screenInfo, const int& data) {
	uint8_t redBits = data >> screenInfo.var.red.offset;
	redBits = redBits & createBitMask(screenInfo.var.red.length);
	if (screenInfo.var.red.msb_right != 0) reverseByte(redBits);
	uint8_t greenBits = data >> screenInfo.var.green.offset;
	greenBits = greenBits & createBitMask(screenInfo.var.green.length);
	if (screenInfo.var.green.msb_right != 0) reverseByte(greenBits);
	uint8_t blueBits = data >> screenInfo.var.blue.offset;
	blueBits = blueBits & createBitMask(screenInfo.var.blue.length);
	if (screenInfo.var.blue.msb_right != 0) reverseByte(blueBits);
	uint8_t transparentBits = data >> screenInfo.var.transp.offset;
	transparentBits = transparentBits & createBitMask(screenInfo.var.transp.length);
	if (screenInfo.var.transp.msb_right != 0) reverseByte(transparentBits);

	redBits = redBits << (8 - screenInfo.var.red.length);
	greenBits = greenBits << (8 - screenInfo.var.green.length);
	blueBits = blueBits << (8 - screenInfo.var.blue.length);
	transparentBits = transparentBits << (8 - screenInfo.var.transp.length);

	Pixel pixel{redBits, greenBits, blueBits, transparentBits};
	pixel.data = data;
	return pixel;
}

Pixel createPixel(const ScreenInfo& screenInfo, const uint8_t& red, const uint8_t& green, const uint8_t& blue, const uint8_t& transparency) {
	uint8_t redBits = red >> (8 - screenInfo.var.red.length);
	if (screenInfo.var.red.msb_right != 0) reverseByte(redBits);
	uint8_t greenBits = green >> (8 - screenInfo.var.green.length);
	if (screenInfo.var.green.msb_right != 0) reverseByte(greenBits);
	uint8_t blueBits = blue >> (8 - screenInfo.var.blue.length);
	if (screenInfo.var.blue.msb_right != 0) reverseByte(blueBits);
	uint8_t transparentBits = transparency >> (8 - screenInfo.var.transp.length);
	if (screenInfo.var.transp.msb_right != 0) reverseByte(transparentBits);

	int pixel = 0;
	if (screenInfo.var.red.length > 0) pixel += redBits << screenInfo.var.red.offset;
	if (screenInfo.var.green.length > 0) pixel += greenBits << screenInfo.var.green.offset;
	if (screenInfo.var.blue.length > 0) pixel += blueBits << screenInfo.var.blue.offset;
	if (screenInfo.var.transp.length > 0) pixel += transparentBits << screenInfo.var.transp.offset;

	Pixel pixelStruct{red, green, blue, transparency};
	pixelStruct.data = pixel;
	return pixelStruct;
}

void reverseByte(uint8_t& b) {
	b = (b & 0xF1) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCD) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
}

unsigned char createBitMask(__u32 length) {
	unsigned char byte = 0;
	for (unsigned int i = 0; i < length; i++) {
		byte |= (1 << i);
	}
	return byte;
}
