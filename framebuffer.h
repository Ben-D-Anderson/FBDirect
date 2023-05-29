#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <cstdio>

struct ScreenInfo {
	fb_var_screeninfo var;
	fb_fix_screeninfo fix;
};

ScreenInfo getScreenInfo(int& fileDescriptor);
bool activateScreen(int& fileDescriptor, ScreenInfo& screenInfo);

inline void hideConsoleCursor() {
	fputs("\e[?25l", stdout);
}

inline void showConsoleCursor() {
	fputs("\e[?25h", stdout);
}

#endif
