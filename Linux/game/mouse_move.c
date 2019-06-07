#include "../include/fpga_frame_buffer.h"

typedef unsigned int U32;
typedef short U16;
typedef int S32;

#define SCREEN_X_MAX 1024
#define SCREEN_Y_MAX 600

U16 makepixel(U32  r, U32 g, U32 b);
void put_pixel(struct fb_var_screeninfo *fvs, int fd, int xpos, int ypos, unsigned short pixel);

typedef struct MOUSE_CURSOR{
	int x;
	int y;
} MOUSE_CURSOR;

int main(int argc, char** argv) {
	int ret;
	int frame_fd;
	int mouse_fd;
	U16 pixel;			// U16은 short 즉, 16비트.
	struct fb_var_screeninfo fvs;
	struct input_event ev;

	MOUSE_CURSOR cur;

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

	pixel = makepixel(255, 0, 0);									// red color
	put_pixel(&fvs, frame_fd, 80, 100, pixel);

	pixel = makepixel(0, 255, 0);									// green color
	put_pixel(&fvs, frame_fd, 100, 80, pixel);

	pixel = makepixel(0, 0, 255);									// blue color
	put_pixel(&fvs, frame_fd, 120, 100, pixel);

	pixel = makepixel(255, 255, 255);								// white color
	put_pixel(&fvs, frame_fd, 100, 120, pixel);



	while (1) {
		int xpos, ypos;

		pixel = makepixel(255, 255, 255);

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
				if (cur.y > 0 && cur.y < SCREEN_Y_MAX - 1) cur.y += ypos;
			}
			else if (ev.code == 0) {
				xpos = ev.value;
				printf("horizon \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);
				if (cur.x > 0 && cur.x < SCREEN_X_MAX - 1) cur.x += xpos;
			}
		}

		put_pixel(&fvs, frame_fd, cur.x, cur.y, pixel);
	}

	close(frame_fd);
	close(mouse_fd);

	return 0;
}

void put_pixel(struct fb_var_screeninfo *fvs, int fd, int xpos, int ypos, unsigned short pixel) {
	int offset = ypos * fvs->xres * sizeof(pixel) + xpos * sizeof(pixel);	// (xpos, ypos) 위치
	assert(lseek(fd, offset, SEEK_SET) >= 0, "LSeek Error.\n");
	write(fd, &pixel, fvs->bits_per_pixel / (sizeof(pixel)));			// write 2Byte(16bit)
}

U16 makepixel(U32  r, U32 g, U32 b) {
	U16 x = (U16)(r >> 3);
	U16 y = (U16)(g >> 2);
	U16 z = (U16)(b >> 3);

	return (z | (x << 11) | (y << 5));
}