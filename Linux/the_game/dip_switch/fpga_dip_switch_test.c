#include "../include/fpga_test.h"

unsigned char quit = 0;
void user_signal(int sig) { quit = 1; }		// 시그널 받으면 호출되는 함수

int main(int argc, char **argv) {
	unsigned char dip_sw_buf = 0;
	int dev;
	int timer = 10;

	dev = open(DIP_SWITCH_DEVICE, O_RDONLY);
	assert2(dev >= 0, "Device open error", DIP_SWITCH_DEVICE);

	(void)signal(SIGINT, user_signal);		// SIGINT 시그널 핸들러 등록
	printf("Press <ctrl+c> to quit.\n");

	while (timer) {
		usleep(1000000);
		read(dev, &dip_sw_buf, &timer);
		printf("Read dip switch: 0x");
		if (dip_sw_buf & 0x80) printf("1");
		else printf("0");
		if (dip_sw_buf & 0x40) printf("1");
		else printf("0");
		if (dip_sw_buf & 0x20) printf("1");
		else printf("0");
		if (dip_sw_buf & 0x10) printf("1");
		else printf("0");
		if (dip_sw_buf & 0x08) printf("1");
		else printf("0");
		if (dip_sw_buf & 0x04) printf("1");
		else printf("0");
		if (dip_sw_buf & 0x02) printf("1");
		else printf("0");
		if (dip_sw_buf & 0x01) printf("1");
		else printf("0");
		printf("\n");
	}

	close(dev);
	return 0;
}