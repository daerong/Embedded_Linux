#include "../include/fpga_frame_buffer.h"
#include "../include/fpga_test.h"
#include "../include/fpga_dot_font.h"

typedef unsigned int U32;
typedef short U16;
typedef int S32;

char keyboard_thread[] = "keyboard thread";

int text_lcd_dev;
unsigned char text_lcd_buf[TEXT_LCD_MAX_BUF];
int text_lcd_locate;

#define SCREEN_X_MAX 1024
#define SCREEN_Y_MAX 600
#define PALETTE_X_END 900
#define TOOLBAR_X_START 901
#define TOOLBAR_X_END 1024

U16 menubox_color;

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
void erase_cursor(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel);

void* keyboard_ev_func(void *data) {
	int keyboard_fd;
	char pnt;						// 키보드 값

	keyboard_fd = open(KEYBOARD_EVENT, O_RDONLY);
	assert2(keyboard_fd >= 0, "Keyboard Event Open Error!", KEYBOARD_EVENT);

	//pthread_mutex_lock(&mutex); // 잠금을 생성한다.

	while (1) {
		struct input_event keyboard_ev;

		if (read(keyboard_fd, &keyboard_ev, sizeof(struct input_event)) < 0)
		{
			printf("check\n");
			if (errno == EINTR)
				continue;

			break;
		}
		if (keyboard_ev.value == 1) {
			if (keyboard_ev.type == 1) {
				switch (keyboard_ev.code) {
				case 1:
					close(keyboard_fd);
					return;
				case 2:
					pnt = '1';
					break;
				case 3:
					pnt = '2';
					break;
				case 4:
					pnt = '3';
					break;
				case 5:
					pnt = '4';
					break;
				case 6:
					pnt = '5';
					break;
				case 7:
					pnt = '6';
					break;
				case 8:
					pnt = '7';
					break;
				case 9:
					pnt = '8';
					break;
				case 10:
					pnt = '9';
					break;
				case 11:
					pnt = '0';
					break;
				case 12:
					pnt = '-';
					break;
				case 13:
					pnt = '=';
					break;
				case 14:
					pnt = '\b';
					break;
				case 15:
					pnt = '\t';
					break;
				case 16:
					pnt = 'q';
					break;
				case 17:
					pnt = 'w';
					break;
				case 18:
					pnt = 'e';
					break;
				case 19:
					pnt = 'r';
					break;
				case 20:
					pnt = 't';
					break;
				case 21:
					pnt = 'y';
					break;
				case 22:
					pnt = 'u';
					break;
				case 23:
					pnt = 'i';
					break;
				case 24:
					pnt = 'o';
					break;
				case 25:
					pnt = 'p';
					break;
				case 26:
					pnt = '[';
					break;
				case 27:
					pnt = ']';
					break;
				case 28:
					unsigned char buffer[TEXT_LCD_LINE_BUF];
					memset(buffer, ' ', TEXT_LCD_LINE_BUF);
					memcpy(buffer, text_lcd_buf + TEXT_LCD_LINE_BUF, TEXT_LCD_LINE_BUF);
					memset(text_lcd_buf, ' ', TEXT_LCD_MAX_BUF);
					memcpy(text_lcd_buf, buffer, TEXT_LCD_LINE_BUF);
					text_lcd_locate = 0;
					break;
				case 30:
					pnt = 'a';
					break;
				case 31:
					pnt = 's';
					break;
				case 32:
					pnt = 'd';
					break;
				case 33:
					pnt = 'f';
					break;
				case 34:
					pnt = 'g';
					break;
				case 35:
					pnt = 'h';
					break;
				case 36:
					pnt = 'j';
					break;
				case 37:
					pnt = 'k';
					break;
				case 38:
					pnt = 'l';
					break;
				case 44:
					pnt = 'z';
					break;
				case 45:
					pnt = 'x';
					break;
				case 46:
					pnt = 'c';
					break;
				case 47:
					pnt = 'v';
					break;
				case 48:
					pnt = 'b';
					break;
				case 49:
					pnt = 'n';
					break;
				case 50:
					pnt = 'm';
					break;
				case 51:
					pnt = ',';
					break;
				case 52:
					pnt = '.';
					break;
				case 53:
					pnt = '/';
					break;
				}

			}

			/*memcpy(text_lcd_buf, "Successful", 10);*/
			if (text_lcd_locate < TEXT_LCD_LINE_BUF) {
				memcpy(text_lcd_buf + TEXT_LCD_LINE_BUF + text_lcd_locate, pnt, 1);
				text_lcd_locate++;
			}

			write(text_lcd_dev, text_lcd_buf, TEXT_LCD_MAX_BUF);

			printf("text : %c \t\t type : %hu, code : %hu, value : %d\n", pnt, keyboard_ev.type, keyboard_ev.code, keyboard_ev.value);

		}
	}

	//pthread_mutex_unlock(&mutex); // 잠금을 해제한다.

	close(keyboard_fd);
}

int main(int argc, char** argv) {
	int ret;
	int frame_fd;
	int mouse_fd;

	memset(text_lcd_buf, ' ', TEXT_LCD_MAX_BUF);
	text_lcd_locate = 0;

	U16 foreground_color;			// U16은 short 즉, 16비트.
	U16 background_color;			// U16은 short 즉, 16비트.
	struct fb_var_screeninfo fvs;
	unsigned short *pfbdata;
	struct input_event mouse_ev;
	pthread_t keyboard_ev_thread;		// pthread
	int thread_id;						// pthread ID
	void *thread_result;				// pthread return
	int status;							// mutex result

	MOUSE_CURSOR cur;
	char draw_mode = 0;
	DISPLAY display[SCREEN_X_MAX * SCREEN_Y_MAX];

	LOCATE start;
	LOCATE end;
	start.xpos = TOOLBAR_X_START;
	start.ypos = 0;
	end.xpos = TOOLBAR_X_END;
	end.ypos = SCREEN_Y_MAX;

	cur.x = SCREEN_X_MAX / 2;
	cur.y = SCREEN_Y_MAX / 2;
	int past_x = cur.x;
	int past_y = cur.y;

	foreground_color = makepixel(255, 255, 255);							// white color
	background_color = makepixel(0, 0, 0);									// black color
	menubox_color = makepixel(50, 150, 150);
	reset_display(&fvs, pfbdata, display, background_color);
	fill_box(&fvs, pfbdata, display, start, end, menubox_color);
	//draw_display(&fvs, pfbdata, display);

	text_lcd_dev = open(TEXT_LCD_DEVICE, O_WRONLY);
	assert2(text_lcd_dev >= 0, "Device open error", TEXT_LCD_DEVICE);

	mouse_fd = open(MOUSE_EVENT, O_RDONLY);
	assert2(mouse_fd >= 0, "Mouse Event Open Error!", MOUSE_EVENT);

	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_VSCREENINFO 사용
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");			// bpp check
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");	// lseek error check

	pfbdata = (unsigned short *)mmap(0, fvs.xres*fvs.yres * sizeof(U16), PROT_READ | PROT_WRITE, MAP_SHARED, frame_fd, 0);
	assert((unsigned)pfbdata != (unsigned)-1, "fbdev mmap error.\n");

	status = pthread_mutex_init(&mutex, NULL);
	assert(status == 0, "Mutex init error.\n");

	while (1) {
		if (read(mouse_fd, &mouse_ev, sizeof(struct input_event)) < 0) {
			printf("check\n");
			if (errno == EINTR)
				continue;

			break;
		}

		if (mouse_ev.type == 1) {
			if (mouse_ev.value == 1) {
				if (mouse_ev.code == 272) {
					if (cur.x < PALETTE_X_END) {
						if (draw_mode) draw_mode = 0;
						else {
							draw_cursor(&fvs, pfbdata, past_x, past_y, background_color);
							draw_mode = 1;
						}
					}
					else {
						thread_id = pthread_create(&keyboard_ev_thread, NULL, keyboard_ev_func, (void *)&keyboard_thread);
						pthread_join(keyboard_ev_thread, (void *)&thread_result);
					}
					
				}
				else if (mouse_ev.code == 273) {
					reset_display(&fvs, pfbdata, display, background_color);
					draw_display(&fvs, pfbdata, display);
				}
			}
		}
		else if (mouse_ev.type == 2) {
			if (mouse_ev.code == 1) {
				cur.y += mouse_ev.value;
			}
			else if (mouse_ev.code == 0) {
				cur.x += mouse_ev.value;
			}
		}
		else {
			//printf("none \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);
		}


		//printf("x : %d \t\t y : %d/n", cur.x, cur.y);

		if (cur.x < 0) {
			cur.x = 0;
		}
		else if (cur.x > SCREEN_X_MAX - 1) {
			cur.x = SCREEN_X_MAX - 1;
		}

		if (cur.y < 0) {
			cur.y = 0;
		}
		else if (cur.y > SCREEN_Y_MAX - 1) {
			cur.y = SCREEN_Y_MAX - 1;
		}

		if (draw_mode) {
			set_pixel(&fvs, pfbdata, display, cur.x, cur.y, foreground_color);
			if(cur.x < PALETTE_X_END) put_pixel(&fvs, pfbdata, cur.x, cur.y, foreground_color);
		}
		else {
			erase_cursor(&fvs, pfbdata, past_x, past_y, background_color);
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
	target[ypos*SCREEN_X_MAX + xpos].color = pixel;
}

void reset_display(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, unsigned short pixel) {
	int x_temp, y_temp;

	for (y_temp = 0; y_temp < SCREEN_Y_MAX; y_temp++) {
		for (x_temp = 0; x_temp < PALETTE_X_END; x_temp++) {
			set_pixel(fvs, pfbdata, target, x_temp, y_temp, pixel);
		}
	}
}

void fill_box(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, LOCATE start, LOCATE end, unsigned short pixel) {
	int x_start, y_start, x_end, y_end;
	int x_temp, y_temp;

	if (start.xpos < 0) x_start = 0;
	else x_start = start.xpos;

	if (start.ypos < 0) y_start = 0;
	else y_start = start.ypos;

	if (end.xpos > SCREEN_X_MAX - 1) x_end = SCREEN_X_MAX - 1;
	else x_end = end.xpos;

	if (end.ypos > SCREEN_Y_MAX - 1) y_end = SCREEN_Y_MAX - 1;
	else y_end = end.ypos;

	for (y_temp = y_start; y_temp < y_end; y_temp++) {
		for (x_temp = x_start; x_temp < x_end; x_temp++) {
			set_pixel(fvs, pfbdata, target, x_temp, y_temp, pixel);
			//target[y_temp*SCREEN_X_MAX + x_temp].color = pixel;
		}
	}
}

void draw_display(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target) {
	int x_temp, y_temp;

	for (y_temp = 0; y_temp < SCREEN_Y_MAX; y_temp++) {
		for (x_temp = 0; x_temp < SCREEN_X_MAX; x_temp++) {
			put_pixel(fvs, pfbdata, x_temp, y_temp, target[y_temp*SCREEN_X_MAX + x_temp].color);
		}
	}
}

void erase_cursor(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel) {
	int i, j;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			if (j + i <= 10) {
				if (ypos + i > SCREEN_Y_MAX - 1 || xpos + j > SCREEN_X_MAX - 1) continue;
				int offset = (ypos + i) * fvs->xres + (xpos + j);
				if(xpos + j < PALETTE_X_END) pfbdata[offset] = pixel;
				else pfbdata[offset] = menubox_color;
			}
		}
	}
	for (i = 5; i < 15; i++) {
		for (j = i; j < i + 3; j++) {
			{
				if (ypos + i > SCREEN_Y_MAX - 1 || xpos + j > SCREEN_X_MAX - 1) continue;
				int offset = (ypos + i) * fvs->xres + (xpos + j);
				if (xpos + j < PALETTE_X_END) pfbdata[offset] = pixel;
				else pfbdata[offset] = menubox_color;
			}
		}
	}
}

void draw_cursor(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel) {
	int i, j;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			if (j + i <= 10) {
				if (ypos + i > SCREEN_Y_MAX - 1 || xpos + j > SCREEN_X_MAX - 1) continue;
				int offset = (ypos + i) * fvs->xres + (xpos + j);
				pfbdata[offset] = pixel;
			}
		}
	}
	for (i = 5; i < 15; i++) {
		for (j = i; j < i + 3; j++) {
			{
				if (ypos + i > SCREEN_Y_MAX - 1 || xpos + j > SCREEN_X_MAX - 1) continue;
				int offset = (ypos + i) * fvs->xres + (xpos + j);
				pfbdata[offset] = pixel;
			}
		}
	}
}