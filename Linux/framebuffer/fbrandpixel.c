#include "../include/fpga_test.h"

/*
4�� ���� : ���� �� ���, Ctrl+C�� ������ �����ϴ� ���� ����
*/

typedef short U16;
U16 random_pixel(void);   // 32��Ʈ ���� ���� ����
void put_pixel(struct fb_var_screeninfo *fvs, int fd, int xpos, int ypos, unsigned short pixel);


int main(int argc, char** argv) {
	int ret;
	int frame_fd;
	U16 rpixel;			// U16�� short ��, 16��Ʈ.
	int offset;
	struct fb_var_screeninfo fvs;

	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo ������ ������ ���� ioctl, FBIOGET_VSCREENINFO ���
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");			// bpp check
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");	// lseek error check

	while (1){
		int xpos, ypos;
		int offset;
		
		xpos = (int)((fvs.xres*1.0*rand()) / (RAND_MAX + 1.0));  // ������ǥ x
		ypos = (int)((fvs.yres*1.0*rand()) / (RAND_MAX + 1.0));  // ������ǥ y
		
		rpixel = random_pixel();

		put_pixel(&fvs, frame_fd, xpos, ypos, rpixel);
	}

	close(frame_fd);
	return 0;
}

U16 random_pixel(void)
{   // ���� ���� �� ��ȯ
	return rand();
}

void put_pixel(struct fb_var_screeninfo *fvs, int fd, int xpos, int ypos, unsigned short pixel) {
	offset = ypos * fvs.xres * sizeof(pixel) + xpos * sizeof(pixel);	// (xpos, ypos) ��ġ
	assert(lseek(fd, offset, SEEK_SET) >= 0, "LSeek Error.\n");
	write(fd, &pixel, fvs.bits_per_pixel / (sizeof(pixel)));			// write 2Byte(16bit)
}