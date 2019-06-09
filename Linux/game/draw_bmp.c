#include "../include/fpga_test.h"

typedef unsigned int U32;
typedef short U16;

U16 makepixel(U32  r, U32 g, U32 b) {
	U16 x = (U16)(r >> 3);
	U16 y = (U16)(g >> 2);
	U16 z = (U16)(b >> 3);

	return (z | (x << 11) | (y << 5));
}

void put_pixel(struct fb_var_screeninfo *fvs, int fd, int xpos, int ypos, unsigned short pixel);

int main(int argc, char** argv) {
	int ret;
	int frame_fd;
	U16 pixel;			// U16은 short 즉, 16비트. 
	struct fb_var_screeninfo fvs;

	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_VSCREENINFO 사용
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");			// bpp check
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");	// lseek error check


	FILE *fp;
	unsigned char info[54];

	fp = fopen("lenna.bmp", "rb");
	if (fp == NULL) {
		printf("File open error: ");
		perror("File open error: ");
		exit(0);
	}

	fread(info, sizeof(unsigned char), 54, fp);

	int width = *(int*)&info[18];
	int height = *(int*)&info[22];

	int size = 3 * width*height; // for RGB

	unsigned char data[size];

	fread(data, sizeof(unsigned char), size, fp);

	int vertical = 0;
	int horizon = 0;
	int locate = 0;

	for (vertical = 0; vertical < height; vertical++) {
		for (horizon = 0; horizon < width; horizon++) {
			locate = vertical * width + horizon;
			pixel = makepixel(data[locate + 2], data[locate], data[locate + 1]);
			put_pixel(&fvs, frame_fd, horizon, width, pixel);
		}
	}



	printf("%d\n", sizeof(pixel));
	close(frame_fd);
	fclose(fp);

	return 0;
}

void put_pixel(struct fb_var_screeninfo *fvs, int fd, int xpos, int ypos, unsigned short pixel) {
	int offset = ypos * fvs->xres * sizeof(pixel) + xpos * sizeof(pixel);	// (xpos, ypos) 위치
	assert(lseek(fd, offset, SEEK_SET) >= 0, "LSeek Error.\n");
	write(fd, &pixel, fvs->bits_per_pixel / (sizeof(pixel)));			// write 2Byte(16bit)
}