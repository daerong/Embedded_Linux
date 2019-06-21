#include "../include/fpga_test.h"

unsigned char quit = 0;
void user_signal(int sig) { quit = 1; }		// 시그널 받으면 호출되는 함수

int main(void) {
	unsigned char state;
	int dev;
	ssize_t ret;

	dev = open(BUZZER_DEVICE, O_RDWR);
	assert2(dev >= 0, "Device open error", BUZZER_DEVICE);

	(void)signal(SIGINT, user_signal);		// SIGINT 시그널 핸들러 등록
	printf("Press <ctrl+c> to quit.\n");

	state = BUZZER_ON;

	while (!quit) {
		state = BUZZER_TOGGLE(state);
		ret = write(dev, &state, 1);
		assert2(ret >= 0, "Device write error", BUZZER_DEVICE);
		sleep(1);
	}

	close(dev);
	return 0;
}
