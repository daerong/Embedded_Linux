#ifndef __FPGA_FRAME_BUFFER_H__
#define __FPGA_FRAME_BUFFER_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <pthread.h>

/* main_lcd */
#include <linux/input.h>
/* main_lcd */

/* framebuffer */
#include <sys/ioctl.h>		// ioctl() �ý��� ��
#include <linux/fb.h>		// Frame Buffer API : fb_var_screeninfo ����ü, fb_fix_screeninfo ����ü, FBIOGET_VSCREENINFO, FBIOGET_FSCREENINFO
#include <sys/types.h>
#include <sys/mman.h>
/* framebuffer */

/* mouse */
#include <errno.h>
#include <stdint.h>
/* mouse */

//touch screen device path
#define TOUCHSCREEN_DEVICE "/dev/input/event1"

//framebuffer
#define LCD_DEVICE "/dev/fb0"

// mouse event
#define MOUSE_EVENT "/dev/input/event9"

void assert(int cond, char *msg) {
	if (!cond) {
		printf("%s\n", msg);
		exit(1);
	}
}

void assert2(int cond, char *msg, char *src) {
	if (!cond) {
		printf("%s: %s\n", msg, src);
		exit(1);
	}
}


#endif