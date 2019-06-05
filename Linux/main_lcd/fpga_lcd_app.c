#include "../include/fpga_frame_buffer.h"

#define X_MAX 1024					// x - resolution : 1024
#define Y_MAX 600					// y - resolution : 600
									// Bit / Pixel : 16
#define FB_LENGTH X_MAX * Y_MAX		// 1024 * 600 = 614400, length of frame buffer memory : 1024 * 600 * 2 = 1228800

char touch_thread[] = "touch thread";

int draw_pointer_x = 0;
int draw_pointer_y = 0;				// 쓰레드간 공유되는 자원
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	// 쓰레드 초기화

typedef unsigned int U32;
typedef short U16;
typedef struct POINT{
	int xpos;
	int ypos;
} POINT;

//typedef enum COLOR_SET {
//
//} COLOR_SET;
//
//typedef enum DRAW_MODE {
//
//} DRAW_MODE;

U16 makepixel(U32 r, U32 g, U32 b);
void put_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel);
void fill_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, POINT left_top, POINT right_bottom, unsigned short pixel);

// Thread Function
void* touch_screen_ev(void *data){
	int touch_dev;
	struct input_event ev;
	size_t ev_size = sizeof(struct input_event);
	size_t size;

	touch_dev = open(TOUCHSCREEN_DEVICE, O_RDWR);
	assert2(touch_dev >= 0, "Device open error", TOUCHSCREEN_DEVICE);

	//pthread_mutex_lock(&mutex); // 잠금을 생성한다.

	while (1) {
		size = read(touch_dev, &ev, ev_size);
		assert(size >= 0, "Touch screen wrong value\n");

		if (ev.value != 0) {
			draw_pointer_x = ev.value;
			size = read(touch_dev, &ev, ev_size);
			assert(size >= 0, "Touch screen wrong value\n");
			if (ev.value != 0) {
				draw_pointer_y = ev.value;
			}
		}
		printf("x : %d, y: %d\n", draw_pointer_x, draw_pointer_y);
	}

	//pthread_mutex_unlock(&mutex); // 잠금을 해제한다.

	close(touch_dev);
}


int main(int argc, char *argv[]) {
	int check, offset;
	int frame_fd;
	U16 pixel;
	int posx1, posy1, posx2, posy2;
	int repx, repy;
	struct fb_var_screeninfo fvs;
	unsigned short *pfbdata;
	pthread_t touch_ev_thread;			
	int thread_id;						// pthread ID
	void *thread_result;				// pthread return
	int status;							// mutex result

	/* Frame buffer setting */
	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	check = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);
	assert(check >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");

	pfbdata = (unsigned short *)mmap(0, fvs.xres*fvs.yres *sizeof(pixel), PROT_READ | PROT_WRITE, MAP_SHARED, frame_fd, 0);
	assert((unsigned)pfbdata != (unsigned)-1, "fbdev mmap error.\n");
	/* Frame buffer setting */

	/* Thread setting */
	status = pthread_mutex_init(&mutex, NULL);
	assert(status == 0, "Mutex init error.\n");

	thread_id = pthread_create(&touch_ev_thread, NULL, touch_screen_ev, (void *)&touch_thread);
	pthread_join(touch_ev_thread, (void *)&thread_result);
	/* Thread setting */

	pixel = makepixel(255, 255, 255); /* white pixel */

	while (1) {
		put_pixel(&fvs, pfbdata, draw_pointer_x, draw_pointer_y, pixel);
	}

	munmap(pfbdata, fvs.xres*fvs.yres * sizeof(pixel));
	close(frame_fd);
	exit(0);
	return 0;
}

U16 makepixel(U32 r, U32 g, U32 b) {
	U16 x = (U16)(r >> 3);
	U16 y = (U16)(g >> 2);
	U16 z = (U16)(b >> 3);

	return (z | (x << 11) | (y << 5));
}

void put_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel) {
	int offset = ypos * fvs->xres + xpos;
	pfbdata[offset] = pixel;
}

void fill_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, POINT one, POINT two, unsigned short pixel) {
	int x_start, x_end, y_start, y_end;
	int x_let, y_let;

	if (one.xpos > two.xpos) {
		x_start = two.xpos;
		x_end = one.xpos
	}
	else {
		x_start = one.xpos;
		x_end = two.xpos
	}
	if (one.ypos > two.ypos) {
		y_start = two.ypos;
		y_end = one.ypos
	}
	else {
		y_start = one.ypos;
		y_end = two.ypos
	}

	for (y_let = y_start; y_let <= y_end; y_let++) {
		for (x_let = x_start; x_let <= x_end; x_let++) {
			put_pixel(fvs, pfbdata, x_let, y_let, pixel);
		}
	}
}