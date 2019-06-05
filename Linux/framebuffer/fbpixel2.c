#include "../include/fpga_test.h"

/*
8번 예제 : mmap을 이용한 pixel 점 찍기.

frame buffer device를 open하고 open이 리턴한 file descriptor를 이용하여 mmap을 합니다.
당연히 offset은 0이고 length는 화면의 해상도와 bpp를 통해 계산된 byte수 만큼이죠. 
mmap이 리턴한 address와 pixel을 찍기를 원하는 좌표를 이용하여 address를 계산하여 pixel을 찍는다.

fbvar.xres*fbvar.yres*(16/8)만큼의 Byte가 mmap으로 virtual address에 매핑되고 있습니다.(당연히 mmap을 지원하지 않는 특이한 frame buffer에서는 에러를 내고 죽게 됩니다.) 
그 address로부터 픽셀이 저장될 address라고 보면 됩니다. 
픽셀이 저장된 순서는 (0,0)부터 시작하여 (1,0), (2,0)의 순서로 (xres-1, 0)까지 진행되고 그 다음 address는 (0, 1)이 되고 다시 (1,1), (2,1),… 이런 식으로 진행됩니다. 
픽셀 offset은 바이트 단위로는 먼저 소개한 fbpixel.c와 같지만 unsigned short 포인터 pfbdata와 더하므로 offset을 바이트 단위로 계산하지 않고 2byte(sizeof(unsigned short))단위로 하고 있습니다. 
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

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_VSCREENINFO 사용
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");			// bpp check
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");	// lseek error check

	pfbdata = (unsigned short *)mmap(0, fvs.xres*fvs.yres * sizeof(pixel), PROT_READ | PROT_WRITE, MAP_SHARED, frame_fd, 0);
	// void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset); : start부터 length까지의 영역을 fd에 대응한다.
	// start : 특별한 경우가 아니면 0, length : 메모리로 매핑할 크기, prot : 맵핑에 원하는 메모리 보호정책, flags : 매핑 유형과 동작 구성 요소, fd : file descriptor, offset : 매핑할 때 length의 시작점을 지정, return : Address

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