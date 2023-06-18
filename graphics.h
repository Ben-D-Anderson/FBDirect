#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "device.h"

void drawLine(Device& device, Pixel& pixel, const unsigned int& startX, const unsigned int& startY, const unsigned int& endX, const unsigned int& endY);
void drawVerticalLine(Device& device, Pixel& pixel, const unsigned int& x, const unsigned int& startY, const unsigned int& endY); 
void drawHorizontalLine(Device& device, Pixel& pixel, const unsigned int& y, const unsigned int& startX, const unsigned int& endX); 
void drawRectangle(Device& device, Pixel& pixel, const unsigned int& startX, const unsigned int& startY, const unsigned int& endX, const unsigned int& endY);
void fillRectangle(Device& device, Pixel& pixel, const unsigned int& startX, const unsigned int& startY, const unsigned int& endX, const unsigned int& endY);

#endif
