#include "../include/fpga_frame_buffer.h"

/*
3번 예제 : 16 bpp 점(pixel) 찍기, write

각 픽셀은 각각의 RGB 값을 가진다. | R(5bit) | G(6bit) | B(5bit) | Red와 Blue는 각각 5비트씩이고 G는 6비트 정보를 가지게 됩니다.
그리고 MSB(Most Significant Bit)에서 LSB(Least Significant Bit)쪽으로 R, G, B 순으로 저장됩니다.

16bpp가 아니면 실행이 되지 않도록 체크 코드가 들어 있습니다.

한 가지 기억해 둘 것은 한 개의 점을 원하는 위치에 찍기 위해서 두 개의 시스템 호출(lseek, write)을 해야 한다는 것입니다.
이것이 원초적이고 무식한(?) 이유입니다. 나중에 배울 새로운 점 찍기 루틴을 보고, 성능 비교를 해보면 왜 무식한지 알 수 있을 겁니다.
*/

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

	int i;
	int vertical = 0;
	int horizon = 0;
	for (i = 0; i < size; i += 3) {
		pixel = makepixel(data[i + 2], data[i], data[i + 1]);
		put_pixel(&fvs, frame_fd, horizon, width, pixel);
		if (horizon < width) horizon++;
		else {
			horizon = 0;
			vertical++;
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