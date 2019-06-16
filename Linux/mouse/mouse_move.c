#include "../include/fpga_test.h"

typedef unsigned int U32;
typedef short U16;
typedef int S32;

char mouse_func_msg[] = "mouse thread";
char chat_func_msg[] = "keyboard thread";

#define SCREEN_X_MAX 1024						// LCD screen의 dot width
#define SCREEN_Y_MAX 600						// LCD screen의 dot height
#define PALETTE_X_END 949						// LCD screen에서 실제적으로 이용하는 부분 0~949
#define TOOLBAR_X_START 950						// LCD screen에서 menu를 표시할 부분 950~1024
#define TOOLBAR_X_END 1024
#define ICON_WIDTH 50							// menu icon의 width
#define ICON_START TOOLBAR_X_START + 12			// menu icon간 가로 space = 12
#define ICON_END ICON_START + ICON_WIDTH
#define ICON_1_Y_START 8						// menu icon간 세로 spcae = 8
#define ICON_2_Y_START 66
#define ICON_3_Y_START 124
#define ICON_4_Y_START 182
#define ICON_5_Y_START 240
#define ICON_6_Y_START 298
#define ICON_7_Y_START 356
#define ICON_8_Y_START 414
#define ICON_9_Y_START 472
#define ICON_10_Y_START 530
										
char text_lcd_mode;								// chat function : keyboard에서 입력한 값을 실시간으로 text lcd에 띄우기 위한 모드 (1 : on, 0 : off)
char camera_mode;
char num_baseball_mode;					
char lenna_img_mode;							// lenna image를 화면에 그리기 (1 : on, 0 : off)

unsigned char *text_lcd_buf;					// text lcd에 실제적으로 쓰여질 buffer

typedef struct DISPLAY {						// screen 색상을 저장할 구조체
	int xpos;
	int ypos;
	U16 color;
} DISPLAY;
typedef struct LOCATE {							// 좌표를 편리하게 다루기 위한 구조체
	int xpos;
	int ypos;
} LOCATE;
typedef struct MOUSE_CURSOR {					// 커서를 지정하기 위한 구조체
	int x;
	int y;
} MOUSE_CURSOR;

U16 makepixel(U32 r, U32 g, U32 b);				// r, g, b 값을 입력해 framebuffer 색상 값을 얻어내는 함수 
void put_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel);		// screen의 (xpos, ypos)위치에 pixel 색상을 가진 점을 찍는 함수
void set_pixel(DISPLAY *target, int xpos, int ypos, unsigned short pixel);				// DISPLAY의 (xpos, ypos) 위치에 색상을 세팅하는 함수
void reset_display(DISPLAY *target, DISPLAY *background);								// DISPLAY 구조체를 copy
void menu_copy(DISPLAY *target, DISPLAY *background);									// DISPLAY 구조체에서 menu 영역만 copy
void fill_box(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, LOCATE start, LOCATE end, unsigned short pixel);		// start~end 영역의 사각형을 pixel 색상으로 채움
void draw_display(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target);		// DISPLAY의 정보를 lcd 화면에 그림
void menu_update(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target);		// menu 부분을 그림
void draw_cursor(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel);							// cursor를 화면에 그림
void erase_cursor(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, DISPLAY *target, DISPLAY *proc_display);		// 이전 좌표의 cursor를 지움
void insert_text_buf(unsigned char *target_buf, int *locate, unsigned char insert_text);		// target_buf 배열의 locate 위치에 insert_text 값을 입력
void set_image(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, int xpos, int ypos, char *file_name);				// file_name 이름을 가진 이미지를 DISPLAY 구조체에 set
void set_small_image(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, int xpos, int ypos, char *file_name);			// 50 x 50 pixel의 이미지가 이상하게 나오는 현상으로 200 x 200 이미지를 샘플링하여 사용함
void erase_image(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *proc_display, DISPLAY *background, int xpos, int ypos, char *file_name);		// 이미지 영역을 지운다.
void* mouse_ev_func(void *data);			// mouse의 이벤트로 LCD screen을 조작하기 위한 thread 생성 시 쓰레드로 사용될 함수
void* chat_func(void *data);				// chat function : keyboard 이벤트 동적 사용 thread

int main(int argc, char** argv) {

	pthread_t mouse_ev_thread;				// mouse event thread의 file descriptor 
	int mouse_thread_id;					// mouse event의 thread ID
	pthread_t keyboard_ev_thread;			// chat function thread의 file descriptor 
	int keyboard_thread_id;					// chat function의 thread ID
	void *thread_result;					// pthread 종료 시 표시할 정보 (ex, 프로세스 number를 지정할 예정)

	text_lcd_mode = 0;				// global variable initialize section
	camera_mode = 0;
	num_baseball_mode = 0;
	lenna_img_mode = 0;

	int text_lcd_dev;													// text lcd에 쓰여질 text buffer
	text_lcd_buf = (unsigned char *)malloc(sizeof(unsigned char)*TEXT_LCD_MAX_BUF);
	memset(text_lcd_buf, ' ', TEXT_LCD_MAX_BUF);

	text_lcd_dev = open(TEXT_LCD_DEVICE, O_WRONLY);						// text lcd의 file descriptor
	assert2(text_lcd_dev >= 0, "Device open error", TEXT_LCD_DEVICE);

	mouse_thread_id = pthread_create(&mouse_ev_thread, NULL, mouse_ev_func, (void *)&mouse_func_msg);			// mouse event thread 생성
	keyboard_thread_id = pthread_create(&keyboard_ev_thread, NULL, chat_func, (void *)&chat_func_msg);			// chat function thread 생성

	while (1) {
		if (text_lcd_mode) {
			write(text_lcd_dev, text_lcd_buf, TEXT_LCD_MAX_BUF);
		}

		sleep(1);
	}

	pthread_join(mouse_ev_thread, (void *)&thread_result);			// mouse event thread 
	pthread_join(keyboard_ev_thread, (void *)&thread_result);

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

void set_pixel(DISPLAY *target, int xpos, int ypos, unsigned short pixel) {
	target[ypos*SCREEN_X_MAX + xpos].color = pixel;
}

void reset_display(DISPLAY *target, DISPLAY *background) {
	memcpy(target, background, sizeof(DISPLAY)*SCREEN_X_MAX*SCREEN_Y_MAX);
}

void menu_copy(DISPLAY *target, DISPLAY *background) {
	int x_temp, y_temp;

	for (y_temp = 0; y_temp < SCREEN_Y_MAX; y_temp++) {
		for (x_temp = TOOLBAR_X_START; x_temp < SCREEN_X_MAX; x_temp++) {
			target[y_temp*SCREEN_X_MAX + x_temp].color = background[y_temp*SCREEN_X_MAX + x_temp].color;
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
			set_pixel(target, x_temp, y_temp, pixel);
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

void menu_update(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target) {
	int x_temp, y_temp;

	for (y_temp = 0; y_temp < SCREEN_Y_MAX; y_temp++) {
		for (x_temp = TOOLBAR_X_START; x_temp < SCREEN_X_MAX; x_temp++) {
			put_pixel(fvs, pfbdata, x_temp, y_temp, target[y_temp*SCREEN_X_MAX + x_temp].color);
		}
	}
}

void erase_cursor(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, DISPLAY *target, DISPLAY *proc_display) {
	U16 pixel;
	int i, j;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			if (j + i <= 10) {
				if (ypos + i > SCREEN_Y_MAX - 1 || xpos + j > SCREEN_X_MAX - 1) continue;
				int offset = (ypos + i) * fvs->xres + (xpos + j);
				pixel = proc_display[offset].color;
				if (xpos + j < PALETTE_X_END) pfbdata[offset] = pixel;
				else pfbdata[offset] = pixel;
			}
		}
	}
	for (i = 5; i < 15; i++) {
		for (j = i; j < i + 3; j++) {
			{
				if (ypos + i > SCREEN_Y_MAX - 1 || xpos + j > SCREEN_X_MAX - 1) continue;
				int offset = (ypos + i) * fvs->xres + (xpos + j);
				pixel = proc_display[offset].color;
				if (xpos + j < PALETTE_X_END) pfbdata[offset] = pixel;
				else pfbdata[offset] = pixel;
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

void insert_text_buf(unsigned char *target_buf, int *locate, unsigned char insert_text) {
	target_buf[*locate] = insert_text;
	(*locate)++;
}

void set_image(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, int xpos, int ypos, char *file_name) {
	U16 pixel;			// U16은 short 즉, 16비트. 
	FILE *fp;

	int width = 0;
	int height = 0;

	unsigned char info [54];

	fp = fopen(file_name, "rb");
	if (fp == NULL) {
		perror("File open error: ");
		exit(0);
	}

	fread(info, sizeof(unsigned char), 54, fp);

	width = *(int*)&info[18];
	height = *(int*)&info[22];

	int size = 3 * width*height; // for RGB

	unsigned char data[size];

	fread(data, sizeof(unsigned char), size, fp);
	fclose(fp);

	int locate = 0;
	int vertical = 0;
	int horizon = 0;


	for (vertical = 0; vertical < height; vertical++) {
		for (horizon = 0; horizon < width; horizon++) {
			locate = (width * height - vertical * width + horizon) * 3;
			pixel = makepixel(data[locate + 2], data[locate + 1], data[locate]);
			set_pixel(target, horizon + xpos, vertical + ypos, pixel);
		}
	}
}

void set_small_image(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *target, int xpos, int ypos, char *file_name) {
	U16 pixel;			// U16은 short 즉, 16비트. 
	FILE *fp;

	int width = 0;
	int height = 0;

	unsigned char info[54];

	fp = fopen(file_name, "rb");
	if (fp == NULL) {
		perror("File open error: ");
		exit(0);
	}

	fread(info, sizeof(unsigned char), 54, fp);

	width = *(int*)&info[18];
	height = *(int*)&info[22];

	int size = 3 * width*height; // for RGB

	unsigned char data[size];

	fread(data, sizeof(unsigned char), size, fp);
	fclose(fp);

	int locate = 0;
	int vertical = 0;
	int horizon = 0;


	for (vertical = 3; vertical < height; vertical += 4) {
		for (horizon = 3; horizon < width; horizon += 4) {
			locate = (width * height - vertical * width + horizon) * 3;
			pixel = makepixel(data[locate + 2], data[locate + 1], data[locate]);
			set_pixel(target, horizon/4 + xpos, vertical/4 + ypos, pixel);
		}
	}
}

void erase_image(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, DISPLAY *proc_display, DISPLAY *background, int xpos, int ypos, char *file_name) {
	FILE *fp;

	int width = 0;
	int height = 0;

	unsigned char info[54];

	fp = fopen(file_name, "rb");
	if (fp == NULL) {
		perror("File open error: ");
		exit(0);
	}

	fread(info, sizeof(unsigned char), 54, fp);

	width = *(int*)&info[18];
	height = *(int*)&info[22];

	fclose(fp);

	int locate = 0;
	int vertical = 0;
	int horizon = 0;


	for (vertical = ypos; vertical < ypos + height; vertical++) {
		for (horizon = xpos; horizon < xpos + width; horizon++) {
			locate = vertical * SCREEN_X_MAX + horizon;
			set_pixel(proc_display, horizon, vertical, background[locate].color);
		}
	}
}

void* mouse_ev_func(void *data) {
	int ret;
	int mouse_fd;
	int frame_fd;
	U16 foreground_color;
	U16 background_color;
	struct fb_var_screeninfo fvs;
	unsigned short *pfbdata;
	struct input_event ev;

	MOUSE_CURSOR cur;
	char draw_mode = 0;
	DISPLAY *display = (DISPLAY *)malloc(sizeof(DISPLAY) * SCREEN_X_MAX * SCREEN_Y_MAX);
	DISPLAY *proc_display = (DISPLAY *)malloc(sizeof(DISPLAY) * SCREEN_X_MAX * SCREEN_Y_MAX);
	DISPLAY *background = (DISPLAY *)malloc(sizeof(DISPLAY) * SCREEN_X_MAX * SCREEN_Y_MAX);

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

	set_image(&fvs, pfbdata, background, 0, 0, "background.bmp");
	reset_display(proc_display, background);

	set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_1_Y_START, "icon1.bmp");
	set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_2_Y_START, "icon2.bmp");
	set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_3_Y_START, "icon3.bmp");
	set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_4_Y_START, "icon4.bmp");
	set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_5_Y_START, "icondefalut.bmp");
	set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_6_Y_START, "icondefalut.bmp");
	set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_7_Y_START, "icondefalut.bmp");
	set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_8_Y_START, "icondefalut.bmp");
	set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_9_Y_START, "icondefalut.bmp");
	set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_10_Y_START, "icondefalut.bmp");
	reset_display(display, proc_display);

	draw_display(&fvs, pfbdata, display);

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
					if (cur.x > TOOLBAR_X_START) {
						if (cur.x >= ICON_START && cur.x < ICON_END) {
							if (cur.y >= ICON_1_Y_START && cur.y < ICON_1_Y_START + ICON_WIDTH) {	// 채팅
								if (text_lcd_mode) {
									set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_1_Y_START, "icon1.bmp");
									menu_copy(display, proc_display);
									menu_update(&fvs, pfbdata, display);
									text_lcd_mode = 0;
								}
								else {
									set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_1_Y_START, "icon1on.bmp");
									menu_copy(display, proc_display);
									menu_update(&fvs, pfbdata, display);
									text_lcd_mode = 1;
								}
							}
							else if (cur.y >= ICON_2_Y_START && cur.y < ICON_2_Y_START + ICON_WIDTH) {	// 카메라
								if (camera_mode) {
									set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_2_Y_START, "icon2.bmp");
									menu_copy(display, proc_display);
									menu_update(&fvs, pfbdata, display);
									camera_mode = 0;
								}
								else {
									set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_2_Y_START, "icon2on.bmp");
									menu_copy(display, proc_display);
									menu_update(&fvs, pfbdata, display);
									camera_mode = 1;
								}
							}
							else if (cur.y >= ICON_3_Y_START && cur.y < ICON_3_Y_START + ICON_WIDTH) {	// 숫자야구
								if (num_baseball_mode) {
									set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_3_Y_START, "icon3.bmp");
									menu_copy(display, proc_display);
									menu_update(&fvs, pfbdata, display);
									num_baseball_mode = 0;
								}
								else {
									set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_3_Y_START, "icon3on.bmp");
									menu_copy(display, proc_display);
									menu_update(&fvs, pfbdata, display);
									num_baseball_mode = 1;
								}
							}
							else if (cur.y >= ICON_4_Y_START && cur.y < ICON_4_Y_START + ICON_WIDTH) {	// lenna image
								if (lenna_img_mode) {
									set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_4_Y_START, "icon4.bmp");
									menu_copy(display, proc_display);
									menu_update(&fvs, pfbdata, display);
									erase_image(&fvs, pfbdata, proc_display, background, 0, 0, "lenna.bmp");
									erase_image(&fvs, pfbdata, display, background, 0, 0, "lenna.bmp");
									draw_display(&fvs, pfbdata, display);
									lenna_img_mode = 0;
								}
								else {
									set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_4_Y_START, "icon4on.bmp");
									menu_copy(display, proc_display);
									menu_update(&fvs, pfbdata, display);
									set_image(&fvs, pfbdata, proc_display, 0, 0, "lenna.bmp");
									set_image(&fvs, pfbdata, display, 0, 0, "lenna.bmp");
									draw_display(&fvs, pfbdata, display);
									lenna_img_mode = 1;
								}
							}
							else if (cur.y >= ICON_5_Y_START && cur.y < ICON_5_Y_START + ICON_WIDTH) {

							}
							else if (cur.y >= ICON_6_Y_START && cur.y < ICON_6_Y_START + ICON_WIDTH) {

							}
							else if (cur.y >= ICON_7_Y_START && cur.y < ICON_7_Y_START + ICON_WIDTH) {

							}
							else if (cur.y >= ICON_8_Y_START && cur.y < ICON_8_Y_START + ICON_WIDTH) {

							}
							else if (cur.y >= ICON_9_Y_START && cur.y < ICON_9_Y_START + ICON_WIDTH) {

							}
							else if (cur.y >= ICON_10_Y_START && cur.y < ICON_10_Y_START + ICON_WIDTH) {

							}
						}
					}
					else {
						if (draw_mode) {
							draw_mode = 0;
						}
						else {
							erase_cursor(&fvs, pfbdata, past_x, past_y, display, proc_display);
							draw_mode = 1;
						}
					}
				}
				else if (ev.code == 273) {
					reset_display(display, proc_display);
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
			if (cur.x < PALETTE_X_END) {
				set_pixel(display, cur.x, cur.y, foreground_color);
				put_pixel(&fvs, pfbdata, cur.x, cur.y, foreground_color);
			}
		}
		else {
			erase_cursor(&fvs, pfbdata, past_x, past_y, display, proc_display);
			draw_cursor(&fvs, pfbdata, cur.x, cur.y, foreground_color);
			past_x = cur.x;
			past_y = cur.y;
		}
	}

	munmap(pfbdata, fvs.xres*fvs.yres * sizeof(U16));
	close(frame_fd);
	close(mouse_fd);

	free(display);
	free(proc_display);
	free(background);

	return 0;
}

void* chat_func(void *data) {
	int keyboard_fd;
	char *inner_text = (char *)malloc(sizeof(char)*TEXT_LCD_LINE_BUF);
	int text_buf_index;

	keyboard_fd = open(KEYBOARD_EVENT, O_RDONLY);
	assert2(keyboard_fd >= 0, "Keyboard Event Open Error!", KEYBOARD_EVENT);

	memset(inner_text, ' ', TEXT_LCD_LINE_BUF);

	while (1) {
		struct input_event ev;

		if (read(keyboard_fd, &ev, sizeof(struct input_event)) < 0) {
			printf("check\n");
			if (errno == EINTR) continue;
			break;
		}
		if (!text_lcd_mode) {

		}
		else if (ev.value == 1) {
			if (ev.type == 1) {
				switch (ev.code) {
				case 1:		// ESC
					memset(inner_text, ' ', TEXT_LCD_LINE_BUF);
					memcpy(text_lcd_buf + TEXT_LCD_LINE_BUF, inner_text, TEXT_LCD_LINE_BUF);
					text_buf_index = 0;
					text_lcd_mode = 0;
					break;
				case 2:
					insert_text_buf(inner_text, &text_buf_index, '1');
					break;
				case 3:
					insert_text_buf(inner_text, &text_buf_index, '2');
					break;
				case 4:
					insert_text_buf(inner_text, &text_buf_index, '3');
					break;
				case 5:
					insert_text_buf(inner_text, &text_buf_index, '4');
					break;
				case 6:
					insert_text_buf(inner_text, &text_buf_index, '5');
					break;
				case 7:
					insert_text_buf(inner_text, &text_buf_index, '6');
					break;
				case 8:
					insert_text_buf(inner_text, &text_buf_index, '7');
					break;
				case 9:
					insert_text_buf(inner_text, &text_buf_index, '8');
					break;
				case 10:
					insert_text_buf(inner_text, &text_buf_index, '9');
					break;
				case 11:
					insert_text_buf(inner_text, &text_buf_index, '0');
					break;
				case 12:
					insert_text_buf(inner_text, &text_buf_index, '-');
					break;
				case 13:
					insert_text_buf(inner_text, &text_buf_index, '=');
					break;
				case 14:	// BACKSPACE
					text_buf_index--;
					insert_text_buf(inner_text, &text_buf_index, ' ');
					text_buf_index--;
					break;
				case 15:	// TAP
					insert_text_buf(inner_text, &text_buf_index, ' ');
					insert_text_buf(inner_text, &text_buf_index, ' ');
					insert_text_buf(inner_text, &text_buf_index, ' ');
					insert_text_buf(inner_text, &text_buf_index, ' ');
					break;
				case 16:
					insert_text_buf(inner_text, &text_buf_index, 'q');
					break;
				case 17:
					insert_text_buf(inner_text, &text_buf_index, 'w');
					break;
				case 18:
					insert_text_buf(inner_text, &text_buf_index, 'e');
					break;
				case 19:
					insert_text_buf(inner_text, &text_buf_index, 'r');
					break;
				case 20:
					insert_text_buf(inner_text, &text_buf_index, 't');
					break;
				case 21:
					insert_text_buf(inner_text, &text_buf_index, 'y');
					break;
				case 22:
					insert_text_buf(inner_text, &text_buf_index, 'u');
					break;
				case 23:
					insert_text_buf(inner_text, &text_buf_index, 'i');
					break;
				case 24:
					insert_text_buf(inner_text, &text_buf_index, 'o');
					break;
				case 25:
					insert_text_buf(inner_text, &text_buf_index, 'p');
					break;
				case 26:
					insert_text_buf(inner_text, &text_buf_index, '[');
					break;
				case 27:
					insert_text_buf(inner_text, &text_buf_index, ']');
					break;
				case 28:	// ENTER
					memcpy(text_lcd_buf, inner_text, TEXT_LCD_LINE_BUF);
					memset(inner_text, ' ', TEXT_LCD_LINE_BUF);
					memcpy(text_lcd_buf + TEXT_LCD_LINE_BUF, inner_text, TEXT_LCD_LINE_BUF);
					text_buf_index = 0;
					break;
				case 30:
					insert_text_buf(inner_text, &text_buf_index, 'a');
					break;
				case 31:
					insert_text_buf(inner_text, &text_buf_index, 's');
					break;
				case 32:
					insert_text_buf(inner_text, &text_buf_index, 'd');
					break;
				case 33:
					insert_text_buf(inner_text, &text_buf_index, 'f');
					break;
				case 34:
					insert_text_buf(inner_text, &text_buf_index, 'g');
					break;
				case 35:
					insert_text_buf(inner_text, &text_buf_index, 'h');
					break;
				case 36:
					insert_text_buf(inner_text, &text_buf_index, 'j');
					break;
				case 37:
					insert_text_buf(inner_text, &text_buf_index, 'k');
					break;
				case 38:
					insert_text_buf(inner_text, &text_buf_index, 'l');
					break;
				case 44:
					insert_text_buf(inner_text, &text_buf_index, 'z');
					break;
				case 45:
					insert_text_buf(inner_text, &text_buf_index, 'x');
					break;
				case 46:
					insert_text_buf(inner_text, &text_buf_index, 'c');
					break;
				case 47:
					insert_text_buf(inner_text, &text_buf_index, 'v');
					break;
				case 48:
					insert_text_buf(inner_text, &text_buf_index, 'b');
					break;
				case 49:
					insert_text_buf(inner_text, &text_buf_index, 'n');
					break;
				case 50:
					insert_text_buf(inner_text, &text_buf_index, 'm');
					break;
				case 51:
					insert_text_buf(inner_text, &text_buf_index, ',');
					break;
				case 52:
					insert_text_buf(inner_text, &text_buf_index, '.');
					break;
				case 53:
					insert_text_buf(inner_text, &text_buf_index, '/');
					break;
				case 57:
					insert_text_buf(inner_text, &text_buf_index, ' ');
					break;
				case 103: //up
					text_buf_index++;
					break;
				case 108: //down
					text_buf_index--;
					break;
				case 106: //right
					text_buf_index++;
					break;

				}
				// 키보드 left 고장

				memcpy(text_lcd_buf + TEXT_LCD_LINE_BUF, inner_text, TEXT_LCD_LINE_BUF);
			}

			// printf("type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);

		}
	}

	free(inner_text);
	close(keyboard_fd);

	return 0;
}