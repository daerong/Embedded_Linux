#include "../include/fpga_test.h"

unsigned char quit = 0;
void user_signal(int sig) { quit = 1; }		// 시그널 받으면 호출되는 함수

int main(void) {
	int dev;					// device handler
	unsigned char push_sw_buf[PUSH_SWITCH_MAX_BUTTON];
	int i;

	dev = open(PUSH_SWITCH_DEVICE, O_RDONLY);
	assert2(dev >= 0, "Device open error", PUSH_SWITCH_DEVICE);

	(void)signal(SIGINT, user_signal);		// SIGINT 시그널 핸들러 등록
	printf("Press <ctrl+c> to quit.\n");

	while (!quit) {
		usleep(400000);
		
		read(dev, &push_sw_buf, sizeof(push_sw_buf));
		for (i = 0; i < PUSH_SWITCH_MAX_BUTTON; i++) {
			printf("[%d] ", push_sw_buf[i]);
		}
		printf("\n");
	}

	close(dev);
	return 0;
}