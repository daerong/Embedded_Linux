#include "../include/fpga_frame_buffer.h"

/*
1번 예제 : LCD 정보를 확인하는 프로그램

우선 수행 중에 생길 수 있는 에러 메시지는 다음과 같습니다.
(1) fbdev open : Permission denied = > root 권한이 없어서 생기는 메시지입니다.
(2) fbdev open : No such device = > frame buffer driver가 제대로 올라가지 않아서 생기는 에러 메시지입니다.
(3) fbdev ioctl…어쩌구 하는 에러 메시지 => frame buffer driver가 정상적인 동작을 하지 않아서 그런 것 같습니다.

수행시키면 다음과 같은 결과를 볼 수 있습니다.
x - resolution : 1024
y - resolution : 600
x - resolution(virtual) : 1024
y - resolution(virtual) : 600
Bit/Pixel : 16
length of frame buffer memory : 1228800
(이 결과는 RedHat 7.3 Permedia2 8MB 그래픽 카드가 있는 시스템에서 보여주는 결과입니다.)
화면 해상도는 1024x600
가상화면 해상도도 1024x600
픽셀당 비트 수는 16이므로 frame buffer memory의 크기는 16MB라고 해석할 수 있다.

가장 중요한 정보는 해상도와 bpp정보
가상화면 정보는 혹시나 해서 출력해 보았고 Frame buffer memory 크기도 크게 중요한 부분은 아닙니다.
*/

int main() {
    int ret;
    int frame_fd;
    struct fb_var_screeninfo st_fvs;		// 프레임 버퍼상에서 바꿀 수 있는 정보
    struct fb_fix_screeninfo st_ffs;		// 프레임 버퍼상에 고정된 정보
    frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);
 
	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &st_fvs);			// fb_var_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_VSCREENINFO 사용
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");
 
	ret = ioctl(frame_fd,FBIOGET_FSCREENINFO,&st_ffs);			// fb_fix_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_FSCREENINFO 사용
	assert(ret >= 0, "Get Information Error - FSCREENINFO!\n");
 
    system("clear");
    printf("==================\n");
    printf("Frame Buffer Info\n");
    printf("------------------\n");
    printf("x - resolution   : %d\n", st_fvs.xres);			// x resolution : 가로 해상도
    printf("y - resolution   : %d\n", st_fvs.yres);			// y resolution : 세로 해상도
    printf("x - resolution(virtual) : %d\n", st_fvs.xres_virtual);	// virtual x resolution : 가상화면 가로 해상도
    printf("y - resolution(virtual) : %d\n", st_fvs.yres_virtual);	// virtual y resolution : 가상화면 세로 해상도
    printf("Bit/Pixel : %d\n", st_fvs.bits_per_pixel);	// bpp : 픽셀당 비트 수
    printf("------------------\n");
    printf("length of frame buffer memory : %d\n", st_ffs.smem_len);		// frame buffer memory : 별로 중요한 부분은 아님
    printf("==================\n");
 
    close(frame_fd);
 
    return 0;
}
