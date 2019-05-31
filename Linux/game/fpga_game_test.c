#include "../include/fpga_test.h"

unsigned char quit = 0;
void user_signal(int sig) { quit = 1; }		// 시그널 받으면 호출되는 함수

int main(void) {
	unsigned char push_sw_buf[PUSH_SWITCH_MAX_BUTTON];
	unsigned char target_num[4] = { 7,5,3,1 };
	unsigned char answer_num[4];
	unsigned char led_data;

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

	(void)signal(SIGINT, user_signal);		// SIGINT 시그널 핸들러 등록
	printf("Press <ctrl+c> to quit.\n");

	target = 0;

	while (!quit) {
		usleep(100000);

		led_data = 0;

		read(push_switch_dev, &push_sw_buf, sizeof(push_sw_buf));
		for (i = 0; i < PUSH_SWITCH_MAX_BUTTON; i++) {
			if (push_sw_buf[i] == 1) {
				answer_num[target] = i + 1;
				if(target < 4) target++;
				else target = 0;
				break;
			}
		}

		ret = write(fnd_dev, answer_num, FND_MAX_DIGIT);
		assert2(ret >= 0, "Device write error", FND_DEVICE);
		usleep(100000);


		if (target_num[3] == answer_num[3]) led_data += 1;
		if (target_num[2] == answer_num[2]) led_data += 2;
		if (target_num[1] == answer_num[1]) led_data += 4;
		if (target_num[0] == answer_num[0]) led_data += 8;

		assert(LED_MIN <= led_data && led_data <= LED_MAX, "Invalid parameter range");


		ret = write(led_dev, &led_data, 1);
		assert2(ret >= 0, "Device write error", LED_DEVICE);
		usleep(100000);
	}

	close(fnd_dev);
	close(led_dev);
	close(push_switch_dev);
	return 0;
}