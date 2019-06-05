#include "../include/fpga_test.h"

/*
9번 예제 : mmap을 이용한 네모 그리기.
*/

typedef short U16;
void swap(int *swapa, int *swapb);  // 박스 좌표 스왑
U16 random_pixel(void);   // 32비트 랜덤 색상값 생성

int main(){
	int check, frame_fd, offset;
	U16 rpixel;
	int posx1, posy1, posx2, posy2;
	int repx, repy;
	struct fb_var_screeninfo fvs;
	unsigned short *pfbdata;
	int count = 1000;

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

	srand(1); /* seed for rand */
	while (0 < count--){
		rpixel = random_pixel();			// 랜덤 색상 값 구함
		posx1 = (int)((fvs.xres*1.0*rand()) / (RAND_MAX + 1.0));  // 랜덤좌표 x1
		posx2 = (int)((fvs.xres*1.0*rand()) / (RAND_MAX + 1.0));  // 랜덤좌표 x2
		posy1 = (int)((fvs.yres*1.0*rand()) / (RAND_MAX + 1.0));  // 랜덤좌표 y1
		posy2 = (int)((fvs.yres*1.0*rand()) / (RAND_MAX + 1.0));  // 랜덤좌표 y2

		swap(&posx1, &posx2);			// 항목당 초기위치 설정
		swap(&posy1, &posy2);			// 가독성상 표현, 조건문 사용이 우선되어야 함.

		for (repy = posy1; repy < posy2; repy++) {		// y set
			offset = repy * fvs.xres;

			for (repx = posx1; repx <= posx2; repx++) {	// x set
				*(pfbdata + offset + repx) = rpixel;
			}
		} // End of For
	}

	munmap(pfbdata, fvs.xres*fvs.yres * sizeof(rpixel));
	close(frame_fd);
	exit(0);
	return 0;
}

U16 random_pixel(void)
{   // 생상값 생성 후 반환
	/*return (int)(65536.0*rand() / (RAND_MAX + 1.0));*/
	return rand();
}

void swap(int *swapa, int *swapb) {
	int temp;
	if (*swapa > *swapb) { // 값 비교후 바꾸어야 된다면 스왑
		temp = *swapb;
		*swapb = *swapa;
		*swapa = temp;
	}
}