#include "../include/fpga_test.h"

unsigned char quit = 0;
//void user_signal(int sig) { quit = 1; }		// 시그널 받으면 호출되는 함수

void usage() {
	printf("Usage:\n");
	printf("\tfpga_step_motor_test <Motor Action> <Motor Diretion> <Speed>\n");
	printf("\tMotor Action : %d(Stop) / %d(Start)\n", STEP_MOTOR_OFF, STEP_MOTOR_ON);
	printf("\tMotor Direction : %d(Left) / %d(Right)\n", STEP_MOTOR_DIR_LEFT, STEP_MOTOR_DIR_RIGHT);
	printf("\tMotor Speed : %d(Fast) ~ %d(Slow)\n", STEP_MOTOR_SPDVAL_MIN, STEP_MOTOR_SPDVAL_MAX);
	printf("\tex) fpga_step_motor_test 1 0 10\n");
}

int main(int argc, char **argv) {
	int dev;					// device handler
	unsigned char state[3];
	//int len;
	int action;				// argv[1]
	int dir;				// argv[2]
	int speed;				// argv[3]

	char sys = 'S';

	action = STEP_MOTOR_ON;
	dir = STEP_MOTOR_DIR_LEFT;
	speed = STEP_MOTOR_SPDVAL_MIN;


	dev = open(STEP_MOTOR_DEVICE, O_WRONLY);
	assert2(dev >= 0, "Device open error", STEP_MOTOR_DEVICE);

	while (sys != 'E') {
		printf("Insert mode (Change speed, Left, Right, End) : ");
		scanf("%c", &sys);

		switch (sys) {
		case 'C':
			printf("Insert speed 0~255, (now : %d) : ", speed);
			scanf("%d", &speed);
			assert(STEP_MOTOR_SPDVAL_MIN <= speed && speed <= STEP_MOTOR_SPDVAL_MAX, "Invalid motor speed");
			break;

		case 'L':
			dir = STEP_MOTOR_DIR_LEFT;
			break;

		case 'R':
			dir = STEP_MOTOR_DIR_RIGHT;
			break;

		default:
			sys = 'E'
		}

		memset(state, 0, sizeof(state));
		state[0] = (unsigned char)action;
		state[1] = (unsigned char)dir;
		state[2] = (unsigned char)speed;

		write(dev, state, 3);
	}

	close(dev);
	return 0;
}