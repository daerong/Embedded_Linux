#ifndef __FPGA_TEST_H__
#define __FPGA_TEST_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>			// open(), close() 시스템 콜
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>			// O_RDWR
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <linux/input.h>
#include <sys/ioctl.h>		// ioctl() 시스템 콜
#include <linux/fb.h>		// Frame Buffer API : fb_var_screeninfo 구조체, fb_fix_screeninfo 구조체, FBIOGET_VSCREENINFO, FBIOGET_FSCREENINFO
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define LEDS_DEVICE "/dev/fpga_led"
#define LEDS_MIN 0
#define LEDS_MAX 255

#define FND_DEVICE "/dev/fpga_fnd"
#define FND_MAX_DIGIT 4

#define DOT_DEVICE "/dev/fpga_dot"
#define DOT_MIN 0
#define DOT_MAX 9

#define TEXT_LCD_DEVICE "/dev/fpga_text_lcd"
#define TEXT_LCD_MAX_BUF 32
#define TEXT_LCD_LINE_BUF 16

#define DIP_SWITCH_DEVICE "/dev/fpga_dip_switch"

#define PUSH_SWITCH_DEVICE "/dev/fpga_push_switch"
#define PUSH_SWITCH_MAX_BUTTON 9

#define BUZZER_DEVICE "/dev/fpga_buzzer"
#define BUZZER_ON 1
#define BUZZER_OFF 0
#define BUZZER_TOGGLE(x) (1-(x))

#define STEP_MOTOR_DEVICE "/dev/fpga_step_motor"
#define STEP_MOTOR_ON 0
#define STEP_MOTOR_OFF 1
#define STEP_MOTOR_DIR_LEFT 0
#define STEP_MOTOR_DIR_RIGHT 1
#define STEP_MOTOR_SPDVAL_MIN 0 // min value, fastest
#define STEP_MOTOR_SPDVAL_MAX 255 // max vlaue, slowest

//touch screen device path
#define TOUCHSCREEN_DEVICE "/dev/input/event1"

//framebuffer
#define LCD_DEVICE "/dev/fb0"

// mouse event
#define MOUSE_EVENT "/dev/input/event9"

// keyboard event
#define KEYBOARD_EVENT "/dev/input/event6"

// tcp-ip
#define MSG_BUF_SIZE 100
#define MAX_CLNT 10
#define MAX_IP 30
#define NORMAL_SIZE 20

// sonic
#define SONIC_DEVICE "/dev/us"


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