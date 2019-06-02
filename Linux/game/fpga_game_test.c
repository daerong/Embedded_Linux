#include "../include/fpga_test.h"
#include "../include/fpga_dot_font.h"

typedef struct STEP_MOTOR {
	int action;
	int dir;	
	int speed;
} STEP_MOTOR;

void motorFunction(unsigned char *data, STEP_MOTOR target) {
	memset(data, 0, sizeof(data));
	data[0] = target.action;
	data[1] = target.dir;
	data[2] = target.speed;
}

unsigned char quit = 0;
void user_signal(int sig) { quit = 1; }		// 시그널 받으면 호출되는 함수

int main(void) {
	unsigned char push_sw_buf[PUSH_SWITCH_MAX_BUTTON];
	unsigned char target_num[4] = { 7,5,3,1 };
	unsigned char answer_num[4];
	unsigned char led_data;
	unsigned char text_lcd_buf[TEXT_LCD_MAX_BUF];
	STEP_MOTOR motor_dat;
	unsigned char motor_data[3];
	unsigned char buzzer_state;
	unsigned char dip_sw_buf;
	unsigned char fan_status;

	//memset(target_num, 0, sizeof(target_num));
	memset(answer_num, 0, sizeof(answer_num));
	memset(text_lcd_buf, ' ', TEXT_LCD_MAX_BUF);
	buzzer_state = BUZZER_ON;
	dip_sw_buf = 0;
	fan_status = 0;

	int fnd_dev;
	int led_dev;
	int push_switch_dev;					// device handler
	int dot_dev;
	int text_lcd_dev;
	int step_motor_dev;
	int buzzer_dev;
	int dip_switch_dev;

	int fan_dev;

	fnd_dev = open(FND_DEVICE, O_RDWR);
	assert2(fnd_dev >= 0, "Device open error", FND_DEVICE);
	led_dev = open(LED_DEVICE, O_RDWR);
	assert2(led_dev >= 0, "Device open error", LED_DEVICE);
	push_switch_dev = open(PUSH_SWITCH_DEVICE, O_RDONLY);
	assert2(push_switch_dev >= 0, "Device open error", PUSH_SWITCH_DEVICE);
	dot_dev = open(DOT_DEVICE, O_WRONLY);
	assert2(dot_dev >= 0, "Device open error", DOT_DEVICE);
	text_lcd_dev = open(TEXT_LCD_DEVICE, O_WRONLY);
	assert2(text_lcd_dev >= 0, "Device open error", TEXT_LCD_DEVICE);
	step_motor_dev = open(STEP_MOTOR_DEVICE, O_WRONLY);
	assert2(step_motor_dev >= 0, "Device open error", STEP_MOTOR_DEVICE);
	buzzer_dev = open(BUZZER_DEVICE, O_RDWR);
	assert2(buzzer_dev >= 0, "Device open error", BUZZER_DEVICE);
	dip_switch_dev = open(DIP_SWITCH_DEVICE, O_RDONLY);
	assert2(dip_switch_dev >= 0, "Device open error", DIP_SWITCH_DEVICE);
	fan_dev = open(FAN_DEVICE, O_RDWR);
	assert2(fan_dev >= 0, "Device open error", FAN_DEVICE);

	int i;
	int target;
	int status;
	int timer;

	ssize_t ret;

	(void)signal(SIGINT, user_signal);		// SIGINT 시그널 핸들러 등록
	printf("Press <ctrl+c> to quit.\n");

	target = 0;
	status = 1;
	timer = 10;

	while (status) {
		usleep(100000);

		led_data = 0;

		read(push_switch_dev, &push_sw_buf, sizeof(push_sw_buf));
		for (i = 0; i < PUSH_SWITCH_MAX_BUTTON; i++) {
			if (push_sw_buf[i] == 1) {
				answer_num[target] = i + 1;
				if (target < 4) target++;
				else target = 0;
				break;
			}
		}

		ret = write(fnd_dev, answer_num, FND_MAX_DIGIT);
		assert2(ret >= 0, "Device write error", FND_DEVICE);
		usleep(100000);


		if (target_num[3] == answer_num[3]) led_data += 16;
		if (target_num[2] == answer_num[2]) led_data += 32;
		if (target_num[1] == answer_num[1]) led_data += 64;
		if (target_num[0] == answer_num[0]) led_data += 128;
		if (led_data == 240) status = 0;

		assert(LED_MIN <= led_data && led_data <= LED_MAX, "Invalid parameter range");


		ret = write(led_dev, &led_data, 1);
		assert2(ret >= 0, "Device write error", LED_DEVICE);
		usleep(100000);
	}

	fan_status = 0xf;
	write(fan_dev, fan_status, 1);

	memcpy(text_lcd_buf, "Successful", 10);
	memcpy(text_lcd_buf + TEXT_LCD_LINE_BUF, "Correct", 7);
	write(text_lcd_dev, text_lcd_buf, TEXT_LCD_MAX_BUF);

	motor_dat.action = 1;
	motor_dat.dir = 0;
	motor_dat.speed = 10;
	motorFunction(motor_data, motor_dat);

	write(step_motor_dev, motor_data, 3);

	read(dip_switch_dev, &dip_sw_buf, 1);

	while (timer--) {

		ret = write(dot_dev, fpga_number[timer % 10], sizeof(fpga_number[timer % 10]));
		assert2(ret >= 0, "Device write error", DOT_DEVICE);

		if (dip_sw_buf) {
			buzzer_state = BUZZER_TOGGLE(buzzer_state);

			ret = write(buzzer_dev, &buzzer_state, 1);
			assert2(ret >= 0, "Device write error", BUZZER_DEVICE);
		}

		sleep(1);
	}

	buzzer_state = BUZZER_OFF;
	ret = write(buzzer_dev, &buzzer_state, 1);
	assert2(ret >= 0, "Device write error", BUZZER_DEVICE);

	motor_dat.action = 0;
	motor_dat.dir = 0;
	motor_dat.speed = 0;
	motorFunction(motor_data, motor_dat);

	write(step_motor_dev, motor_data, 3);

	fan_status = 0x0;
	write(fan_dev, fan_status, 1);

	close(fnd_dev);
	close(led_dev);
	close(push_switch_dev);
	close(dot_dev);
	close(text_lcd_dev);
	close(step_motor_dev);
	close(buzzer_dev);
	close(dip_switch_dev);
	close(fan_dev);

	return 0;
}