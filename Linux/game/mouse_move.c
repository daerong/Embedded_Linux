#include "../include/fpga_frame_buffer.h"

typedef unsigned int U32;
typedef short U16;
typedef int S32;

#define SCREEN_X_MAX 1024
#define SCREEN_Y_MAX 600

typedef struct POINT {
	int xpos;
	int ypos;
} POINT;
typedef struct MOUSE_CURSOR {
	int x;
	int y;
} MOUSE_CURSOR;

U16 makepixel(U32  r, U32 g, U32 b);
void put_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel);
void fill_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, POINT one, POINT two, unsigned short pixel);



int main(int argc, char** argv) {
	int ret;
	int frame_fd;
	int mouse_fd;
	U16 pixel;			// U16은 short 즉, 16비트.
	struct fb_var_screeninfo fvs;
	unsigned short *pfbdata;
	struct input_event ev;

	MOUSE_CURSOR cur;
	POINT start;
	POINT end;
	start.xpos = 0;
	start.ypos = 0;
	end.xpos = SCREEN_X_MAX - 1;
	end.ypos = SCREEN_Y_MAX - 1;

	cur.x = fvs.xres / 2;
	cur.y = fvs.yres / 2;

	mouse_fd = open(MOUSE_EVENT, O_RDONLY);
	assert2(frame_fd >= 0, "Mouse Event Open Error!", MOUSE_EVENT);

	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_VSCREENINFO 사용
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");			// bpp check
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");	// lseek error check

	pfbdata = (unsigned short *)mmap(0, fvs.xres*fvs.yres * sizeof(pixel), PROT_READ | PROT_WRITE, MAP_SHARED, frame_fd, 0);
	assert((unsigned)pfbdata != (unsigned)-1, "fbdev mmap error.\n");

	pixel = makepixel(0, 0, 0);									// black color
	fill_pixel(&fvs, pfbdata, start, end, pixel)



	while (1) {
		int xpos, ypos;

		if (read(mouse_fd, &ev, sizeof(struct input_event)) < 0)
		{
			printf("check\n");
			if (errno == EINTR)
				continue;

			break;
		}

		if (ev.type == 1) {
			if (ev.value == 1) {
				if (ev.code == 272) printf("left btn \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);
				else if (ev.code == 273) printf("right btn \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);
			}
		}
		else if (ev.type == 2) {
			if (ev.code == 1) {
				ypos = ev.value;
				printf("vertical \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);
				cur.y += ypos;
			}
			else if (ev.code == 0) {
				xpos = ev.value;
				printf("horizon \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);
				cur.x += xpos;
			}
		}


		printf("x : %d \t\t y : %d/n", cur.x, cur.y);

		pixel = makepixel(255, 255, 255);
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

		put_pixel(&fvs, pfbdata, cur.x, cur.y, pixel);
	}


	munmap(pfbdata, fvs.xres*fvs.yres * sizeof(pixel));
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

void fill_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, POINT one, POINT two, unsigned short pixel) {
	int x_start, x_end, y_start, y_end;
	int x_let, y_let;

	if (one.xpos > two.xpos) {
		x_start = two.xpos;
		x_end = one.xpos;
	}
	else {
		x_start = one.xpos;
		x_end = two.xpos;
	}
	if (one.ypos > two.ypos) {
		y_start = two.ypos;
		y_end = one.ypos;
	}
	else {
		y_start = one.ypos;
		y_end = two.ypos;
	}

	for (y_let = y_start; y_let <= y_end; y_let++) {
		for (x_let = x_start; x_let <= x_end; x_let++) {
			put_pixel(fvs, pfbdata, x_let, y_let, pixel);
		}
	}
}