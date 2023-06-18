#include "graphics.h"
#include <cstdlib>

void drawLine(Device& device, Pixel& pixel, const unsigned int& startX, const unsigned int& startY, const unsigned int& endX, const unsigned int& endY) {
	unsigned int dx = endX - startX;
	unsigned int dy = endY - startY;
	unsigned int x = startX, y = startY;
	unsigned int steps;
	float xIncrement, yIncrement;
	if (abs(dx) > abs(dy)) {
		steps = abs(dx);
	} else {
		steps = abs(dy);
	}
	xIncrement = dx / (float) steps;
	yIncrement = dy / (float) steps;
	device.setBufferPixel(x, y, pixel);
	for (int v = 0; v < steps; v++) {
		x += xIncrement;
		y += yIncrement;
		device.setBufferPixel(x, y, pixel);
	}
}

void drawVerticalLine(Device& device, Pixel& pixel, const unsigned int& x, const unsigned int& startY, const unsigned int& endY) {
	for (unsigned int y = startY; y <= endY; y++) {
		device.setBufferPixel(x, y, pixel);
	}
}

void drawHorizontalLine(Device& device, Pixel& pixel, const unsigned int& y, const unsigned int& startX, const unsigned int& endX) {
	for (unsigned int x = startX; x <= endX; x++) {
		device.setBufferPixel(x, y, pixel);
	}
}

void drawRectangle(Device& device, Pixel& pixel, const unsigned int& startX, const unsigned int& startY, const unsigned int& endX, const unsigned int& endY) {
	drawHorizontalLine(device, pixel, startY, startX, endX);
	drawHorizontalLine(device, pixel, endY, startX, endX);
	drawVerticalLine(device, pixel, startX, startY, endY);
	drawVerticalLine(device, pixel, endX, startY, endY);
}

void fillRectangle(Device& device, Pixel& pixel, const unsigned int& startX, const unsigned int& startY, const unsigned int& endX, const unsigned int& endY) {
	for (unsigned int y = startY; y <= endY; y++) {
		drawHorizontalLine(device, pixel, y, startX, endX);
	}
}
