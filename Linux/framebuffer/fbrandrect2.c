#include "../include/fpga_test.h"

/*
9�� ���� : mmap�� �̿��� �׸� �׸���.
*/

typedef short U16;
void swap(int *swapa, int *swapb);  // �ڽ� ��ǥ ����
U16 random_pixel(void);   // 32��Ʈ ���� ���� ����

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

	ret = ioctl(frame_fd, FBIOGET_VSCREENINFO, &fvs);		// fb_var_screeninfo ������ ������ ���� ioctl, FBIOGET_VSCREENINFO ���
	assert(ret >= 0, "Get Information Error - VSCREENINFO!\n");

	assert(fvs.bits_per_pixel == 16, "bpp is not 16\n");			// bpp check
	assert(lseek(frame_fd, 0, SEEK_SET) >= 0, "LSeek Error.\n");	// lseek error check

	pfbdata = (unsigned short *)mmap(0, fvs.xres*fvs.yres * sizeof(pixel), PROT_READ | PROT_WRITE, MAP_SHARED, frame_fd, 0);
	// void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset); : start���� length������ ������ fd�� �����Ѵ�.
	// start : Ư���� ��찡 �ƴϸ� 0, length : �޸𸮷� ������ ũ��, prot : ���ο� ���ϴ� �޸� ��ȣ��å, flags : ���� ������ ���� ���� ���, fd : file descriptor, offset : ������ �� length�� �������� ����, return : Address

	assert((unsigned)pfbdata != (unsigned)-1, "fbdev mmap error.\n");

	srand(1); /* seed for rand */
	while (0 < count--){
		rpixel = random_pixel();			// ���� ���� �� ����
		posx1 = (int)((fvs.xres*1.0*rand()) / (RAND_MAX + 1.0));  // ������ǥ x1
		posx2 = (int)((fvs.xres*1.0*rand()) / (RAND_MAX + 1.0));  // ������ǥ x2
		posy1 = (int)((fvs.yres*1.0*rand()) / (RAND_MAX + 1.0));  // ������ǥ y1
		posy2 = (int)((fvs.yres*1.0*rand()) / (RAND_MAX + 1.0));  // ������ǥ y2

		swap(&posx1, &posx2);			// �׸�� �ʱ���ġ ����
		swap(&posy1, &posy2);			// �������� ǥ��, ���ǹ� ����� �켱�Ǿ�� ��.

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
{   // ���� ���� �� ��ȯ
	/*return (int)(65536.0*rand() / (RAND_MAX + 1.0));*/
	return rand();
}

void swap(int *swapa, int *swapb) {
	int temp;
	if (*swapa > *swapb) { // �� ���� �ٲپ�� �ȴٸ� ����
		temp = *swapb;
		*swapb = *swapa;
		*swapa = temp;
	}
}