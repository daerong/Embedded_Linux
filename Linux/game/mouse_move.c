#include "../include/fpga_frame_buffer.h"

typedef unsigned int U32;
typedef short U16;
typedef int S32;

#define SCREEN_X_MAX 900
#define SCREEN_Y_MAX 600
#define TOOLBAR_START 901
#define TOOLBAR_END 1024

typedef struct DISPLAY {
	int xpos;
	int ypos;
	U16 color;
} DISPLAY;
typedef struct LOCATE {
	int xpos;
	int ypos;
} LOCATE;
typedef struct MOUSE_CURSOR {
	int x;
	int y;
} MOUSE_CURSOR;

U16 makepixel(U32  r, U32 g, U32 b);
void put_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel);
void set_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, int xpos, int ypos, unsigned short pixel);
void reset_display(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, unsigned short pixel);
void fill_box(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, LOCATE start, LOCATE end, unsigned short pixel);
void draw_display(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target);
void draw_cursor(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel);

int main(int argc, char** argv) {
	int ret;
	int frame_fd;
	int mouse_fd;
	U16 foreground_color;			// U16은 short 즉, 16비트.
	U16 background_color;			// U16은 short 즉, 16비트.
	U16 menubox_color;
	struct fb_var_screeninfo fvs;
	unsigned short *pfbdata;
	struct input_event ev;
	LOCATE start;
	LOCATE end;
	start.xpos = TOOLBAR_START;
	start.ypos = 0;
	end.xpos = TOOLBAR_END;
	end.ypos = SCREEN_Y_MAX;

	MOUSE_CURSOR cur;
	char draw_mode = 0;
	DISPLAY display[TOOLBAR_END * SCREEN_Y_MAX];
	foreground_color = makepixel(255, 255, 255);							// white color
	background_color = makepixel(0, 0, 0);									// black color
	menubox_color = makepixel(100, 100, 100);
	reset_display(&fvs, pfbdata, display, background_color);
	fill_box(&fvs, pfbdata, display, start, end, menubox_color);

	cur.x = TOOLBAR_END / 2;
	cur.y = SCREEN_Y_MAX / 2;
	int past_x = cur.x;
	int past_y = cur.y;

	mouse_fd = open(MOUSE_EVENT, O_RDONLY);
	assert2(frame_fd >= 0, "Mouse Event Open Error!", MOUSE_EVENT);

	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_VSCREENINFO 사용
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");			// bpp check
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");	// lseek error check

	pfbdata = (unsigned short *)mmap(0, fvs.xres*fvs.yres * sizeof(U16), PROT_READ | PROT_WRITE, MAP_SHARED, frame_fd, 0);
	assert((unsigned)pfbdata != (unsigned)-1, "fbdev mmap error.\n");

	while (1) {
		if (read(mouse_fd, &ev, sizeof(struct input_event)) < 0) {
			printf("check\n");
			if (errno == EINTR)
				continue;

			break;
		}

		if (ev.type == 1) {
			if (ev.value == 1) {
				if (ev.code == 272) {
					if (draw_mode) draw_mode = 0;
					else {
						draw_cursor(&fvs, pfbdata, past_x, past_y, background_color);
						draw_mode = 1;
					}
				}
				else if (ev.code == 273) {
					reset_display(&fvs, pfbdata, display, background_color);
					draw_display(&fvs, pfbdata, display);
				}
			}
		}
		else if (ev.type == 2) {
			if (ev.code == 1) {
				cur.y += ev.value;
			}
			else if (ev.code == 0) {
				cur.x += ev.value;
			}
		}
		else {
			//printf("none \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);
		}


		//printf("x : %d \t\t y : %d/n", cur.x, cur.y);

		if (cur.x < 0) {
			cur.x = 0;
		}
		else if (cur.x > TOOLBAR_END - 1) {
			cur.x = TOOLBAR_END - 1;
		}

		if (cur.y < 0) {
			cur.y = 0;
		}
		else if (cur.y > SCREEN_Y_MAX - 1) {
			cur.y = SCREEN_Y_MAX - 1;
		}

		if (draw_mode) {
			set_pixel(&fvs, pfbdata, display, cur.x, cur.y, foreground_color);
			put_pixel(&fvs, pfbdata, cur.x, cur.y, foreground_color);
		}
		else {
			draw_cursor(&fvs, pfbdata, past_x, past_y, background_color);
			draw_cursor(&fvs, pfbdata, cur.x, cur.y, foreground_color);
			past_x = cur.x;
			past_y = cur.y;
		}
	}


	munmap(pfbdata, fvs.xres*fvs.yres * sizeof(U16));
	close(frame_fd);
	close(mouse_fd);

	return 0;
}

U16 makepixel(U32  r, U32 g, U32 b) {
	U16 x = (U16)(r >> 3);
	U16 y = (U16)(g >> 2);
	U16 z = (U16)(b >> 3);

	return (z | (x << 11) | (y << 5));
}

void put_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel) {
	int offset = ypos * fvs->xres + xpos;
	pfbdata[offset] = pixel;
}

void set_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, int xpos, int ypos, unsigned short pixel) {
	if (xpos > SCREEN_X_MAX) return;
	target[ypos*TOOLBAR_END + xpos].color = pixel;
}

void reset_display(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, unsigned short pixel) {
	int x_temp, y_temp;

	for (y_temp = 0; y_temp < SCREEN_Y_MAX; y_temp++) {
		for (x_temp = 0; x_temp < SCREEN_X_MAX; x_temp++) {
			set_pixel(fvs, pfbdata, target, x_temp, y_temp, pixel);
		}
	}
}

void fill_box(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, LOCATE start, LOCATE end, unsigned short pixel) {
	int x_temp, y_temp;

	for (y_temp = start.ypos; y_temp < end.ypos; y_temp++) {
		for (x_temp = start.xpos; x_temp < end.xpos; x_temp++) {
			set_pixel(fvs, pfbdata, target, x_temp, y_temp, pixel);
		}
	}
}

void draw_display(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target) {
	int x_temp, y_temp;

	for (y_temp = 0; y_temp < SCREEN_Y_MAX; y_temp++) {
		for (x_temp = 0; x_temp < TOOLBAR_END; x_temp++) {
			put_pixel(fvs, pfbdata, x_temp, y_temp, target[y_temp*TOOLBAR_END + x_temp].color);
		}
	}
}

void draw_cursor(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel) {
	int i, j;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			if (j + i <= 10) {
				if (ypos + i > SCREEN_Y_MAX - 1 || xpos + j > TOOLBAR_END - 1) continue;
				int offset = (ypos + i) * fvs->xres + (xpos + j);
				pfbdata[offset] = pixel;
			}
		}
	}
	for (i = 5; i < 15; i++) {
		for (j = i; j < i + 3; j++) {
			{
				if (ypos + i > SCREEN_Y_MAX - 1 || xpos + j > TOOLBAR_END - 1) continue;
				int offset = (ypos + i) * fvs->xres + (xpos + j);
				pfbdata[offset] = pixel;
			}
		}
	}
}