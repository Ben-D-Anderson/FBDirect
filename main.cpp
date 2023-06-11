#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>

#include "device.h"
#include "graphics.h"

void demo_colourChangingScreen(Device& device, ScreenInfo& info) {
	for (unsigned int i = 0; i < 256; i++) {
		Pixel pixel = createPixel(info, 255-i, i, 255-i, 255);
		for (unsigned int y = 0; y < device.getHeight(); y++) {
			for (unsigned int x = 0; x < device.getWidth(); x++) {
				device.setBufferPixel(x, y, pixel);
			}
		}
		device.renderBuffer();
	}
}

void demo_gradualScreenFill(Device& device, ScreenInfo& info) {
	Pixel pixel = createPixel(info, 255, 255, 255, 255);
	for (int i = 0; i < device.getWidth(); i++) {
		drawLine(device, pixel, i, 0, i, device.getHeight() - 1);
		device.renderBuffer();
		usleep(3000);
	}
}

int main() {
	hideConsoleCursor();

	const char* devicePath = "/dev/fb0";
	int fd = open(devicePath, O_RDWR);
	ScreenInfo info{getScreenInfo(fd)};
	activateScreen(fd, info);

	Device device{info, std::string(devicePath)};
	device.mapToMemory();

	auto begin = std::chrono::high_resolution_clock::now();
	
	demo_colourChangingScreen(device, info);
	demo_gradualScreenFill(device, info);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	printf("Time measured: %.3f seconds.\n", elapsed.count() * 1e-9);

	device.blankBuffer();
	device.renderBuffer();

	device.unmapFromMemory();
	close(fd);
	showConsoleCursor();

	return 0;
}
