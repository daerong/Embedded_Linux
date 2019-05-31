#include "../include/fpga_test.h"

unsigned char quit = 0;
void user_signal(int sig) { quit = 1; }		// �ñ׳� ������ ȣ��Ǵ� �Լ�

int main(void) {
	unsigned char push_sw_buf[PUSH_SWITCH_MAX_BUTTON];
	unsigned char target_num[4] = { 7,5,3,1 };
	unsigned char answer_num[4];

	//memset(target_num, 0, sizeof(target_num));
	memset(answer_num, 0, sizeof(answer_num));

	int fnd_dev;
	int led_dev;
	int push_switch_dev;					// device handler

	fnd_dev = open(FND_DEVICE, O_RDWR);
	assert2(fnd_dev >= 0, "Device open error", FND_DEVICE);
	led_dev = open(LED_DEVICE, O_RDWR);
	assert2(led_dev >= 0, "Device open error", LED_DEVICE);
	push_switch_dev = open(PUSH_SWITCH_DEVICE, O_RDONLY);
	assert2(push_switch_dev >= 0, "Device open error", PUSH_SWITCH_DEVICE);

	int i;
	int target;

	ssize_t ret;

	(void)signal(SIGINT, user_signal);		// SIGINT �ñ׳� �ڵ鷯 ���
	printf("Press <ctrl+c> to quit.\n");

	while (!quit) {
		usleep(100000);

		target = 0;

		read(push_switch_dev, &push_sw_buf, sizeof(push_sw_buf));
		for (i = 0; i < PUSH_SWITCH_MAX_BUTTON; i++) {
			if (push_sw_buf[i] == 1) {
				target = i;
				answer_num[3] = target;
				break;
			}
		}

		ret = write(fnd_dev, answer_num, FND_MAX_DIGIT);
		assert2(ret >= 0, "Device write error", FND_DEVICE);
		sleep(1);

	}

	close(fnd_dev);
	close(led_dev);
	close(push_switch_dev);
	return 0;
}