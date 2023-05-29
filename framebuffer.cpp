#include "framebuffer.h"

ScreenInfo getScreenInfo(int& fileDescriptor) {
	fb_var_screeninfo vinfo{};
	if (ioctl(fileDescriptor, FBIOGET_VSCREENINFO, &vinfo) == -1) {
		perror("Error reading variable screen info");
	}
	fb_fix_screeninfo finfo{};
	if (ioctl(fileDescriptor, FBIOGET_FSCREENINFO, &finfo) == -1) {
		perror("Error reading fixed screen info");
	}
	return ScreenInfo{vinfo, finfo};
}

bool activateScreen(int& fileDescriptor, ScreenInfo& screenInfo) {
	screenInfo.var.activate |= FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
	if(ioctl(fileDescriptor, FBIOPUT_VSCREENINFO, &screenInfo.var) == -1) {
		perror("Error activing frame buffer");
		return false;
	}
	return true;
}
