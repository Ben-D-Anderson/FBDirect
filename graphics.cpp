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
