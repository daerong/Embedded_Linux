#include "../include/fpga_frame_buffer.h"

/*
2�� ���� : bits_per_pixel �ʵ带 �ٲ� �� �Ѱ��ִ� ���α׷�.

�켱 ���� �߿� ���� �� �ִ� ���� �޽����� ������ �����ϴ�.
(1) fbdev open : Permission denied = > root ������ ��� ����� �޽����Դϴ�.
(2) fbdev open : No such device = > frame buffer driver�� ����� �ö��� �ʾƼ� ����� ���� �޽����Դϴ�.
(3) fbdev ioctl����¼�� �ϴ� ���� �޽��� = > frame buffer driver�� �������� ������ ���� �ʾƼ� �׷� �� �����ϴ�.
(4) fbdev ioctl(PUT) : Invalid argument => �� �޽����� frame buffer device�� 16 bpp�� �ٲ� �� ���ٴ� �޽����Դϴ�. �ϵ��� �����ϴ� �ػ󵵿� bpp�� �Ѱ谡 ���� �� �ִ�.

RedHat 8.0 ATI mach 64 CT 2MB �ý����� ��� ������ ��������.atyfb�� ������ ������ ���Դϴ�.
fbinfo�� ������� yres_virtual�� ��û���� �̻��� ���� ������ �ְ� ���� �װ��� �ٽ� 480���� �ٲ��Ŀ� FBIOPUT_VSCREENINFO�� ���ָ� ������ �ذ�˴ϴ�.

�� ���α׷��� �������� �����ϰ� �� ������ ó���� ���α׷��� �ٽ� �����ϸ� ������ ���� ����� �� �� �ֽ��ϴ�
x - resolution : 640
y - resolution : 480
x - resolution(virtual) : 640
y - resolution(virtual) : 480
Bit / Pixel : 16
length of frame buffer memory : 8388608
(�� ����� RedHat 7.3 Permedia2 8MB �׷��� ī�尡 �ִ� �ý��ۿ��� �����ִ� ����Դϴ�.)

bpp�� ����� �ٲ�����ϴ�.�������� ���� ��ȭ�� �����ϴ�. 
���� �ػ󵵸� 800x600���� �ٲٴ� ���α׷��� ��, 32bpp�� 24bpp�� �ٲ� ���� �ְ���.(�� �ϵ����� frame buffer driver�� ������ �Ѵٸ�.)

== ��ġ��
�� ���ĺ��� ����Ǵ� ��� �ۿ� �ִ� �ҽ��ڵ�� 16bpp�� �������� �ۼ��� �����Դϴ�.
���� PDA�� Handphone�� 16bpp�� ���� �ְ� StrongARM���� ���� 16bpp LCD�� �ٴ� �߼��̹Ƿ� 16bpp�� ũ�� ������ �Ǹ��� ���� �ʽ��ϴ�.
*/

int main(){
	int frame_fd;
	int ret;
	struct fb_var_screeninfo fvs;

	frame_fd = open(LCD_DEVICE, O_RDWR);
	assert2(frame_fd >= 0, "Frame Buffer Open Error!", LCD_DEVICE);

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo ������ ������ ���� ioctl, FBIOGET_VSCREENINFO ���
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	fvs.bits_per_pixel = 16;

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo ������ ������ ���� ioctl, FBIOGET_VSCREENINFO ���
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	close(frame_fd);
	exit(0);
	return 0;
}