#include "../include/fpga_test.h"

/*
4번 예제 : 랜덤 점 찍기, Ctrl+C를 누르면 중지하는 무한 루프
*/

typedef short U16;
U16 random_pixel(void);   // 32비트 랜덤 색상값 생성
void put_pixel(struct fb_var_screeninfo *fvs, int fd, int xpos, int ypos, unsigned short pixel);


int main(int argc, char** argv) {
	int ret;
	int frame_fd;
	U16 rpixel;			// U16은 short 즉, 16비트.
	int offset;
	struct fb_var_screeninfo fvs;

	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_VSCREENINFO 사용
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");			// bpp check
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");	// lseek error check

	while (1){
		int xpos, ypos;
		int offset;
		
		xpos = (int)((fvs.xres*1.0*rand()) / (RAND_MAX + 1.0));  // 랜덤좌표 x
		ypos = (int)((fvs.yres*1.0*rand()) / (RAND_MAX + 1.0));  // 랜덤좌표 y
		
		rpixel = random_pixel();

		put_pixel(&fvs, frame_fd, xpos, ypos, rpixel);
	}

	close(frame_fd);
	return 0;
}

U16 random_pixel(void)
{   // 생상값 생성 후 반환
	return rand();
}

void put_pixel(struct fb_var_screeninfo *fvs, int fd, int xpos, int ypos, unsigned short pixel) {
	offset = ypos * fvs.xres * sizeof(pixel) + xpos * sizeof(pixel);	// (xpos, ypos) 위치
	assert(lseek(fd, offset, SEEK_SET) >= 0, "LSeek Error.\n");
	write(fd, &pixel, fvs.bits_per_pixel / (sizeof(pixel)));			// write 2Byte(16bit)
}