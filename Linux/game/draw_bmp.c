#include "../include/fpga_frame_buffer.h"

/*
3�� ���� : 16 bpp ��(pixel) ���, write

�� �ȼ��� ������ RGB ���� ������. | R(5bit) | G(6bit) | B(5bit) | Red�� Blue�� ���� 5��Ʈ���̰� G�� 6��Ʈ ������ ������ �˴ϴ�.
�׸��� MSB(Most Significant Bit)���� LSB(Least Significant Bit)������ R, G, B ������ ����˴ϴ�.

16bpp�� �ƴϸ� ������ ���� �ʵ��� üũ �ڵ尡 ��� �ֽ��ϴ�.

�� ���� ����� �� ���� �� ���� ���� ���ϴ� ��ġ�� ��� ���ؼ� �� ���� �ý��� ȣ��(lseek, write)�� �ؾ� �Ѵٴ� ���Դϴ�.
�̰��� �������̰� ������(?) �����Դϴ�. ���߿� ��� ���ο� �� ��� ��ƾ�� ����, ���� �񱳸� �غ��� �� �������� �� �� ���� �̴ϴ�.
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
	U16 pixel;			// U16�� short ��, 16��Ʈ. 
	struct fb_var_screeninfo fvs;

	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo ������ ������ ���� ioctl, FBIOGET_VSCREENINFO ���
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
	int offset = ypos * fvs->xres * sizeof(pixel) + xpos * sizeof(pixel);	// (xpos, ypos) ��ġ
	assert(lseek(fd, offset, SEEK_SET) >= 0, "LSeek Error.\n");
	write(fd, &pixel, fvs->bits_per_pixel / (sizeof(pixel)));			// write 2Byte(16bit)
}