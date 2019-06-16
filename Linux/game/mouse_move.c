#include "../include/fpga_test.h"

typedef unsigned int U32;
typedef short U16;
typedef int S32;

char mouse_func_msg[] = "mouse thread";
char chat_func_msg[] = "keyboard thread";
char tcp_ip_func_msg[] = "tcp ip thread";
char sonic_func_msg[] = "sonic thread";
char *socket_ext_msg = "exit";

int sonic_fd;
int sonic_buf;

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
#define MY_NAME "Yoo"

U16 menubox_color;
char text_lcd_mode;								// chat function : keyboard에서 입력한 값을 실시간으로 text lcd에 띄우기 위한 모드 (1 : on, 0 : off)
char camera_mode;
char num_baseball_mode;
char lenna_img_mode;							// lenna image를 화면에 그리기 (1 : on, 0 : off)

unsigned char *text_lcd_buf;					// text lcd에 실제적으로 쓰여질 buffer

char make_thread;
char delete_thread;
char icon_off;
char send_msg_stat;
char recv_msg_stat;

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
char u16_to_char(short target);				// short 변수를 char변수로 변환
void* mouse_ev_func(void *data);			// mouse의 이벤트로 LCD screen을 조작하기 위한 thread 생성 시 쓰레드로 사용될 함수
void* chat_func(void *data);				// chat function : keyboard 이벤트 동적 사용 thread
void* sonic_func(void *data);
void* write_sonic_func(void *data);
/* tcp function */
void* send_msg(void* arg);					// tcp/ip에서 사용할 send thread
void* recv_msg(void* arg);					// tcp/ip에서 사용할 receive thread
void error_handling(char* msg);
void menu();								// tcp/ip에 대한 내용을 띄어줄 함수

char name[NORMAL_SIZE] = "[DEFALT]";		// name
char msg_form[NORMAL_SIZE];					// msg form
char serv_time[NORMAL_SIZE];				// server time
char msg[MSG_BUF_SIZE];						// msg
char serv_port[NORMAL_SIZE];				// server port number
char clnt_ip[NORMAL_SIZE];					// client ip address
/* tcp function */

int main(int argc, char* argv[]) {
	int text_lcd_dev;
	pthread_t mouse_ev_thread;				// mouse event thread의 file descriptor 
	int mouse_thread_id;					// pthread ID
	pthread_t chat_thread;
	int chat_thread_id;						// pthread ID
	pthread_t sonic_thread;
	int sonic_thread_id;						// pthread ID
	pthread_t tcp_id_thread;
	int tcp_id_thread_id;						// pthread ID
	void *thread_result;				// pthread return
	int status;							// mutex result
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void* thread_return;
	char usage[50];

	sprintf(usage, "Usage : %s <ip> <port> <name>\n", argv[0]);
	assert(argc == 4, usage);

	/* local time */
	struct tm *t;
	time_t timer = time(NULL);
	t = localtime(&timer);
	sprintf(serv_time, "%d-%d-%d %d:%d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
	/* local time */

	sprintf(name, "[%s]", argv[3]);
	sprintf(clnt_ip, "%s", argv[1]);
	sprintf(serv_port, "%s", argv[2]);

	text_lcd_mode = 0;
	camera_mode = 0;
	num_baseball_mode = 0;
	lenna_img_mode = 0;

	make_thread = 0;
	delete_thread = 0;
	icon_off = 0;
	send_msg_stat = 0;
	recv_msg_stat = 0;

	sock = socket(PF_INET, SOCK_STREAM, 0);

	text_lcd_dev = open(TEXT_LCD_DEVICE, O_WRONLY);
	assert2(text_lcd_dev >= 0, "Device open error", TEXT_LCD_DEVICE);

	text_lcd_buf = (unsigned char *)malloc(sizeof(unsigned char)*TEXT_LCD_MAX_BUF);
	memset(text_lcd_buf, ' ', TEXT_LCD_MAX_BUF);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	mouse_thread_id = pthread_create(&mouse_ev_thread, NULL, mouse_ev_func, (void *)&mouse_func_msg);

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
		error_handling(" conncet() error");
	}

	menu();
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);

	while (1) {
		if (recv_msg_stat) {
			write(text_lcd_dev, text_lcd_buf, TEXT_LCD_MAX_BUF);
			recv_msg_stat = 0;
		}
		else if (text_lcd_mode) {
			write(text_lcd_dev, text_lcd_buf, TEXT_LCD_MAX_BUF);
		}

		switch (make_thread) {
		case 1:
			chat_thread_id = pthread_create(&chat_thread, NULL, chat_func, (void *)&chat_func_msg);
			printf("%s\n", chat_func_msg);
			make_thread = 0;
			break;
		case 5:
			sonic_thread_id = pthread_create(&sonic_thread, NULL, sonic_func, (void *)&sonic_func_msg);
			printf("%s\n", sonic_func_msg);
			make_thread = 0;
		}

		switch (delete_thread) {
		case 1:
			pthread_join(chat_thread, (void *)&thread_result);
			icon_off = 1;
			printf("thread %d down.\n", *((int *)thread_result));
			delete_thread = 0;
			break;
		}

	}

	pthread_join(mouse_ev_thread, (void *)&thread_result);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);
	free(text_lcd_buf);
	close(text_lcd_dev);

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
			set_pixel(target, horizon / 4 + xpos, vertical / 4 + ypos, pixel);
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

char u16_to_char(short target) {
	switch (target) {
	case 1:
		return 'Q';			// ESC
	case 2:
		return '1';
	case 3:
		return '2';
	case 4:
		return '3';
	case 5:
		return '4';
	case 6:
		return '5';
	case 7:
		return '6';
	case 8:
		return '7';
	case 9:
		return '8';
	case 10:
		return '9';
	case 11:
		return '0';
	case 12:
		return '-';
	case 13:
		return '=';
	case 14:
		return 'B';			// BACKSPACE
	case 15:
		return 'T';			// TAP
	case 16:
		return 'q';
	case 17:
		return 'w';
	case 18:
		return 'e';
	case 19:
		return 'r';
	case 20:
		return 't';
	case 21:
		return 'y';
	case 22:
		return 'u';
	case 23:
		return 'i';
	case 24:
		return 'o';
	case 25:
		return 'p';
	case 26:
		return '[';
	case 27:
		return ']';
	case 28:
		return 'E';			// ENTER
	case 30:
		return 'a';
	case 31:
		return 's';
	case 32:
		return 'd';
	case 33:
		return 'f';
	case 34:
		return 'g';
	case 35:
		return 'h';
	case 36:
		return 'j';
	case 37:
		return 'k';
	case 38:
		return 'l';
	case 44:
		return 'z';
	case 45:
		return 'x';
	case 46:
		return 'c';
	case 47:
		return 'v';
	case 48:
		return 'b';
	case 49:
		return 'n';
	case 50:
		return 'm';
	case 51:
		return ',';
	case 52:
		return '.';
	case 53:
		return '/';
	case 57:
		return ' ';
	case 103:
		return 'U';			 // Up
	case 108:
		return 'D';			 // Down
	case 106:
		return 'R';			 // Right
	}
	// 키보드 left 고장

	return '?';
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
	menubox_color = makepixel(50, 150, 150);

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

	fill_box(&fvs, pfbdata, background, start, end, menubox_color);
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
		switch (icon_off) {
		case 1:
			set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_1_Y_START, "icon1.bmp");
			menu_copy(display, proc_display);
			menu_update(&fvs, pfbdata, display);
			icon_off = 0;
			break;
		}

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
								if (!text_lcd_mode) {
									set_small_image(&fvs, pfbdata, proc_display, ICON_START, ICON_1_Y_START, "icon1on.bmp");
									menu_copy(display, proc_display);
									menu_update(&fvs, pfbdata, display);
									make_thread = 1;
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
							else if (cur.y >= ICON_5_Y_START && cur.y < ICON_5_Y_START + ICON_WIDTH) {		// sonic
								make_thread = 5;
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
	int text_buf_index = 0;
	char changed_char;
	static int retval = 1;			// 종료되는 프로세스 번호

	memset(inner_text, ' ', TEXT_LCD_LINE_BUF);

	keyboard_fd = open(KEYBOARD_EVENT, O_RDONLY);
	assert2(keyboard_fd >= 0, "Keyboard Event Open Error!", KEYBOARD_EVENT);

	while (1) {
		struct input_event ev;

		if (read(keyboard_fd, &ev, sizeof(struct input_event)) < 0) {
			printf("check\n");
			if (errno == EINTR) continue;
			break;
		}
		if (!text_lcd_mode) {
			break;
		}
		else if (ev.value == 1) {
			if (ev.type == 1) {
				changed_char = u16_to_char(ev.code);
				switch (changed_char) {
				case 'Q':		// ESC
					memset(inner_text, ' ', TEXT_LCD_LINE_BUF);
					memcpy(text_lcd_buf + TEXT_LCD_LINE_BUF, inner_text, TEXT_LCD_LINE_BUF);
					text_buf_index = 0;
					text_lcd_mode = 0;
					break;
				case 'B':		// BACKSPACE
					text_buf_index--;
					insert_text_buf(inner_text, &text_buf_index, ' ');
					text_buf_index--;
					break;
				case 'T':		// TAP
					insert_text_buf(inner_text, &text_buf_index, ' ');
					insert_text_buf(inner_text, &text_buf_index, ' ');
					insert_text_buf(inner_text, &text_buf_index, ' ');
					insert_text_buf(inner_text, &text_buf_index, ' ');
					break;
				case 'E':		// ENTER
					memcpy(text_lcd_buf, inner_text, TEXT_LCD_LINE_BUF);
					text_buf_index = 0;
					send_msg_stat = 1;
					break;
				case 'U':		// Up
					if (text_buf_index < TEXT_LCD_LINE_BUF - 1) text_buf_index++;
					break;
				case 'D':		// Down
					if (text_buf_index > 0) text_buf_index--;
					break;
				case 'R':		// Right
					text_buf_index++;
					break;
				default:
					insert_text_buf(inner_text, &text_buf_index, changed_char);
				}
				if (text_lcd_mode) {
					memcpy(text_lcd_buf + TEXT_LCD_LINE_BUF, inner_text, TEXT_LCD_LINE_BUF);
				}

			}

			//printf("%c", pnt);

			//printf("type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);

		}
	}

	delete_thread = 1;
	free(inner_text);
	close(keyboard_fd);
	pthread_exit((void*)&retval);

	return 0;
}

void* sonic_func(void *data) {

	pthread_t write_sonic_thread;
	int write_sonic_thread_id;						// pthread ID
	write_sonic_thread_id = pthread_create(&write_sonic_thread, NULL, write_sonic_func, (void *)&sonic_func_msg);

	int retn;
	int loop = 0;
	sonic_fd = open("/dev/us", O_RDWR);
	printf("fd = %d\n", sonic_fd);
	if (sonic_fd < 0) {
		perror("/dev/us error");
		exit(-1);
	}
	else {
		printf("< us device has been detected >\n");
	}
	while (1) {
		read(sonic_fd, &buf, 2);
		printf("distance user : %d (cm)\n", buf);
		usleep(200000);
	}
	close(fd);
	return 0;


}

void* write_sonic_func(void *data) {
	write(sonic_fd, &buf, 2);
	for (loop = 0; loop < 100000; loop++) {};
}

void* send_msg(void* arg) {
	int sock = *((int*)arg);
	char name_msg[NORMAL_SIZE + MSG_BUF_SIZE];
	char *clean_text = (char *)malloc(sizeof(char)*TEXT_LCD_LINE_BUF);
	char* who = NULL;
	char temp[MSG_BUF_SIZE];

	char myInfo[MSG_BUF_SIZE];
	/** send join messge **/
	printf(" >> join the chat !! \n");
	sprintf(myInfo, "%s's join. IP_%s\n", name, clnt_ip);
	write(sock, myInfo, strlen(myInfo));

	while (1){
		if (send_msg_stat) {
			strncpy(msg, text_lcd_buf + TEXT_LCD_LINE_BUF, TEXT_LCD_LINE_BUF);
			memset(clean_text, ' ', TEXT_LCD_LINE_BUF);
			memcpy(text_lcd_buf + TEXT_LCD_LINE_BUF, clean_text, TEXT_LCD_LINE_BUF);

			// send message
			memset(name_msg, ' ', NORMAL_SIZE + MSG_BUF_SIZE);
			sprintf(name_msg, "%s %s\n", name, msg);
			write(sock, (void*)&name_msg, sizeof(name_msg));
			send_msg_stat = 0;
		}

	}
	return NULL;
}

void* recv_msg(void* arg)
{
	int sock = *((int*)arg);
	char name_msg[NORMAL_SIZE + MSG_BUF_SIZE];
	char tar = '[';
	int str_len;
	int locate = 0;;

	while (1){
		memset(name_msg, ' ', NORMAL_SIZE + MSG_BUF_SIZE);
		str_len = read(sock, name_msg, NORMAL_SIZE + MSG_BUF_SIZE - 1);
		if (str_len == -1)
			return (void*)-1;
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);

		if (str_len < TEXT_LCD_LINE_BUF) {
			memset(text_lcd_buf, ' ', TEXT_LCD_LINE_BUF);
			strncpy(text_lcd_buf, name_msg, str_len - 1);
		}
		else {
			/*char *ptr1;
			ptr1 = strrchr(name_msg, tar);
			if (ptr1) {
				locate = ptr1 - name_msg;
			}


			printf("%d %d : %s\n", str_len, locate, name_msg);
			strncpy(text_lcd_buf, name_msg + locate, TEXT_LCD_LINE_BUF);*/
		}

		recv_msg_stat = 1;
	}
	return NULL;
}


void menu()
{
	system("clear");
	printf(" **** moon/sum chatting client ****\n");
	printf(" server port : %s \n", serv_port);
	printf(" client IP   : %s \n", clnt_ip);
	printf(" chat name   : %s \n", name);
	printf(" server time : %s \n", serv_time);
	printf(" ************* menu ***************\n");
	printf(" if you want to select menu -> !menu\n");
	printf(" 1. change name\n");
	printf(" 2. clear/update\n");
	printf(" **********************************\n");
	printf(" Exit -> q & Q\n\n");
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
