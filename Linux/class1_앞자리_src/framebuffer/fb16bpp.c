#include "../include/fpga_frame_buffer.h"

/*
2번 예제 : bits_per_pixel 필드를 바꾼 후 넘겨주는 프로그램.

우선 수행 중에 생길 수 있는 에러 메시지는 다음과 같습니다.
(1) fbdev open : Permission denied = > root 권한이 없어서 생기는 메시지입니다.
(2) fbdev open : No such device = > frame buffer driver가 제대로 올라가지 않아서 생기는 에러 메시지입니다.
(3) fbdev ioctl…어쩌구 하는 에러 메시지 = > frame buffer driver가 정상적인 동작을 하지 않아서 그런 것 같습니다.
(4) fbdev ioctl(PUT) : Invalid argument => 이 메시지는 frame buffer device가 16 bpp로 바꿀 수 없다는 메시지입니다. 하드웨어가 지원하는 해상도와 bpp에 한계가 있을 수 있다.

RedHat 8.0 ATI mach 64 CT 2MB 시스템의 경우 에러가 나더군요.atyfb의 버그인 것으로 보입니다.
fbinfo의 결과에서 yres_virtual이 엄청나게 이상한 값을 가지고 있고 따라서 그것을 다시 480으로 바꾼후에 FBIOPUT_VSCREENINFO를 해주면 문제가 해결됩니다.

이 프로그램을 에러없이 수행하고 난 다음에 처음의 프로그램을 다시 수행하면 다음과 같은 결과를 볼 수 있습니다
x - resolution : 640
y - resolution : 480
x - resolution(virtual) : 640
y - resolution(virtual) : 480
Bit / Pixel : 16
length of frame buffer memory : 8388608
(이 결과는 RedHat 7.3 Permedia2 8MB 그래픽 카드가 있는 시스템에서 보여주는 결과입니다.)

bpp가 제대로 바뀌었습니다.나머지는 물론 변화가 없습니다. 
이제 해상도를 800x600으로 바꾸는 프로그램도 또, 32bpp나 24bpp로 바꿀 수도 있겠죠.(단 하드웨어와 frame buffer driver가 지원만 한다면.)

== 마치며
이 이후부터 진행되는 모든 글에 있는 소스코드는 16bpp를 기준으로 작성할 예정입니다.
요즘 PDA나 Handphone도 16bpp가 많이 있고 StrongARM에도 보통 16bpp LCD를 다는 추세이므로 16bpp가 크게 문제가 되리라 보지 않습니다.
*/

int main(){
	int frame_fd;
	int ret;
	struct fb_var_screeninfo fvs;

	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_VSCREENINFO 사용
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	fvs.bits_per_pixel = 16;

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo 정보를 얻어오기 위해 ioctl, FBIOGET_VSCREENINFO 사용
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	close(frame_fd);
	exit(0);
	return 0;
}