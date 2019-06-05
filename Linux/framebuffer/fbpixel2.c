#include "../include/fpga_test.h"

/*
8�� ���� : mmap�� �̿��� pixel �� ���.

frame buffer device�� open�ϰ� open�� ������ file descriptor�� �̿��Ͽ� mmap�� �մϴ�.
�翬�� offset�� 0�̰� length�� ȭ���� �ػ󵵿� bpp�� ���� ���� byte�� ��ŭ����. 
mmap�� ������ address�� pixel�� ��⸦ ���ϴ� ��ǥ�� �̿��Ͽ� address�� ����Ͽ� pixel�� ��´�.

fbvar.xres*fbvar.yres*(16/8)��ŭ�� Byte�� mmap���� virtual address�� ���εǰ� �ֽ��ϴ�.(�翬�� mmap�� �������� �ʴ� Ư���� frame buffer������ ������ ���� �װ� �˴ϴ�.) 
�� address�κ��� �ȼ��� ����� address��� ���� �˴ϴ�. 
�ȼ��� ����� ������ (0,0)���� �����Ͽ� (1,0), (2,0)�� ������ (xres-1, 0)���� ����ǰ� �� ���� address�� (0, 1)�� �ǰ� �ٽ� (1,1), (2,1),�� �̷� ������ ����˴ϴ�. 
�ȼ� offset�� ����Ʈ �����δ� ���� �Ұ��� fbpixel.c�� ������ unsigned short ������ pfbdata�� ���ϹǷ� offset�� ����Ʈ ������ ������� �ʰ� 2byte(sizeof(unsigned short))������ �ϰ� �ֽ��ϴ�. 
*/

typedef unsigned char ubyte;
typedef unsigned int U32;
typedef short U16;

U16 makepixel(U32 r, U32 g, U32 b) {
	U16 x = (U16)(r >> 3);
	U16 y = (U16)(g >> 2);
	U16 z = (U16)(b >> 3);

	return (z | (x << 11) | (y << 5));
}

void put_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel);

int main(){
	int check, frame_fd, offset;
	U16 pixel;
	int posx1, posy1, posx2, posy2;
	int repx, repy;
	struct fb_var_screeninfo fvs;

	unsigned short *pfbdata;

	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo ������ ������ ���� ioctl, FBIOGET_VSCREENINFO ���
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");			// bpp check
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");	// lseek error check

	pfbdata = (unsigned short *)mmap(0, fvs.xres*fvs.yres * sizeof(pixel), PROT_READ | PROT_WRITE, MAP_SHARED, frame_fd, 0);
	// void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset); : start���� length������ ������ fd�� �����Ѵ�.
	// start : Ư���� ��찡 �ƴϸ� 0, length : �޸𸮷� ������ ũ��, prot : ���ο� ���ϴ� �޸� ��ȣ��å, flags : ���� ������ ���� ���� ���, fd : file descriptor, offset : ������ �� length�� �������� ����, return : Address

	assert((unsigned)pfbdata != (unsigned)-1, "fbdev mmap error.\n");

	/* red pixel @ (0,0) */
	pixel = makepixel(255, 0, 0); /* red pixel */
	put_pixel(&fvs, pfbdata, 0, 0, pixel);

	/* green pixel @ (100,50) */
	pixel = makepixel(0, 255, 0); /* green pixel */
	put_pixel(&fvs, pfbdata, 100, 50, pixel);

	/* blue pixel @ (50,100) */
	pixel = makepixel(0, 0, 255); /* blue pixel */
	put_pixel(&fvs, pfbdata, 50, 100, pixel);

	/* white pixel @ (100,100) */
	pixel = makepixel(255, 255, 255); /* white pixel */
	put_pixel(&fvs, pfbdata, 100, 100, pixel);

	munmap(pfbdata, fvs.xres*fvs.yres * sizeof(pixel));
	close(frame_fd);
	exit(0);
	return 0;
}

void put_pixel(struct fb_var_screeninfo *fvs, unsigned short *pfbdata, int xpos, int ypos, unsigned short pixel) {
	int offset = ypos * fvs->xres + xpos;
	pfbdata[offset] = pixel;
}