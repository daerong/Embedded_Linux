#include "../include/fpga_test.h"
 
/*
5번 예제 : 랜덤위치 박스 찍기, Ctrl+C를 누르면 중지하는 무한 루프

lseek, read를 이용하면 원하는 위치의 pixel값을 읽어올 수도 있다는 예측도 해보았습니다.
*/

typedef short U16;
void swap(int *swapa, int *swapb);  // 박스 좌표 스왑
U16 random_pixel(void);   // 32비트 랜덤 색상값 생성
 
int main(int argc, char** argv) {
    int check, frame_fd, offset; 
    U16 rpixel;
	int posx1, posy1, posx2, posy2;
	int repx, repy;
    struct fb_var_screeninfo fvs;
 
	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	check = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);				// fb_var_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_VSCREENINFO 사용
	assert(check >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");			// bpp check
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");	// lseek error check

    while(1) {
		rpixel = random_pixel();			// 랜덤 색상 값 구함
		posx1 = (int)((fvs.xres*1.0*rand()) / (RAND_MAX + 1.0));  // 랜덤좌표 x1
		posx2 = (int)((fvs.xres*1.0*rand()) / (RAND_MAX + 1.0));  // 랜덤좌표 x2
		posy1 = (int)((fvs.yres*1.0*rand()) / (RAND_MAX + 1.0));  // 랜덤좌표 y1
		posy2 = (int)((fvs.yres*1.0*rand()) / (RAND_MAX + 1.0));  // 랜덤좌표 y2

        swap(&posx1, &posx2);			// 항목당 초기위치 설정
        swap(&posy1, &posy2);			// 가독성상 표현, 조건문 사용이 우선되어야 함.
 
        usleep(500000);					// 0.5초 대기 
 
		for (repy = posy1; repy < posy2; repy++) {		// y set
			offset = repy * fvs.xres * (sizeof(rpixel)) + posx1 * (sizeof(rpixel));
			assert(lseek(frame_fd, offset, SEEK_SET) >= 0, "LSeek Error.\n");

			for (repx = posx1; repx <= posx2; repx++) {	// x set
				write(frame_fd, &rpixel, (sizeof(rpixel)));
			}
        } // End of For
    } // End of While

    close(frame_fd);
    return 0;
}
 
U16 random_pixel(void)
{   // 생상값 생성 후 반환
    return rand();
}
 
void swap(int *swapa, int *swapb) {
    int temp; 
    if(*swapa > *swapb) { // 값 비교후 바꾸어야 된다면 스왑
        temp = *swapb;
        *swapb = *swapa;
        *swapa = temp;
    }
}