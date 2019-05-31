#include "../include/fpga_test.h"

#include <termios.h>
#include <stdio.h>

#define ENTER 13
#define ESCAPE 27
#define BACK_SPACE 8

#define ZERO 48
#define ONE 49
#define TWO 50
#define THREE 51
#define FOUR 52
#define FIVE 53
#define SIX 54
#define SEVEN 55
#define EIGHT 56
#define NINE 57

#define KEY_ZERO 96
#define KEY_ONE 97
#define KEY_TWO 98
#define KEY_THREE 99
#define KEY_FOUR 100
#define KEY_FIVE 101
#define KEY_SIX 102
#define KEY_SEVEN 103
#define KEY_EIGHT 104
#define KEY_NINE 105

#define LEFT 37
#define UP 38
#define RIGHT 39
#define DOWN 40



static struct termios before, after;

/* Initialize after terminal i/o settings */
void initTermios(int echo)
{
	tcgetattr(0, &before); /* grab before terminal i/o settings */
	after = before; /* make after settings same as before settings */
	after.c_lflag &= ~ICANON; /* disable buffered i/o */
	if (echo) {
		after.c_lflag |= ECHO; /* set echo mode */
	}
	else {
		after.c_lflag &= ~ECHO; /* set no echo mode */
	}
	tcsetattr(0, TCSANOW, &after); /* use these after terminal i/o settings now */
}

/* Restore before terminal i/o settings */
void resetTermios(void)
{
	tcsetattr(0, TCSANOW, &before);
}

/* Read 1 character - echo defines echo mode */
unsigned char getch_(int echo)
{
	unsigned char ch;
	initTermios(echo);
	ch = getchar();
	resetTermios();
	return ch;
}

/* Read 1 character without echo */
char getch(void)
{
	return getch_(0);
}

/* Read 1 character with echo */
unsigned char getche(void)
{
	return getch_(1);
}

void fndUpdate(int dev, char *data) {
	ssize_t ret;

	ret = write(dev, data, FND_MAX_DIGIT);
	assert2(ret >= 0, "Device write error", FND_DEVICE);
	sleep(1);
}

void fndReset(int dev) {
	unsigned char data[4];
	memset(data, 0, sizeof(data));
	ssize_t ret;

	ret = write(dev, &data, FND_MAX_DIGIT);
	assert2(ret >= 0, "Device write error", FND_DEVICE);
	sleep(1);
}

int sameCheck(unsigned char *num_arr, int index, int repeat){
	int i;
	for(i = 0; i<repeat; i++){
		if(i==index) continue;
		
		if(num_arr[i] == num_arr[index]) return 1;
	}

	return 0;
}

int setNum(int dev, unsigned char *num_arr, int size) {
	if (size != 4) return 1;

	int status = 1;
	unsigned char temp;
	int index = 3;

	while (status) {
		while (temp != ENTER) {
			temp = getche();
			switch (temp) {
			case LEFT:
				if (index > 0) index--;
				break;
			case RIGHT:
				if (index < 3) index++;
				break;
			case ZERO:
			case KEY_ZERO:
				num_arr[index] = 0;
				if (sameCheck(num_arr, index, size)) num_arr[index] = 0;
				else if (index > 0) index--;		
				break;
			case ONE:
			case KEY_ONE:
				num_arr[index] = 1;
				if (sameCheck(num_arr, index, size)) num_arr[index] = 0;
				else if (index > 0) index--;	
				break;
			case TWO:
			case KEY_TWO:
				num_arr[index] = 2;
				if (sameCheck(num_arr, index, size)) num_arr[index] = 0;
				else if (index > 0) index--;	
				break;
			case THREE:
			case KEY_THREE:
				num_arr[index] = 3;
				if (sameCheck(num_arr, index, size)) num_arr[index] = 0;
				else if (index > 0) index--;	
				break;
			case FOUR:
			case KEY_FOUR:
				num_arr[index] = 4;
				if (sameCheck(num_arr, index, size)) num_arr[index] = 0;
				else if (index > 0) index--;	
				break;
			case FIVE:
			case KEY_FIVE:
				num_arr[index] = 5;
				if (sameCheck(num_arr, index, size)) num_arr[index] = 0;
				else if (index > 0) index--;	
				break;
			case SIX:
			case KEY_SIX:
				num_arr[index] = 6;
				if (sameCheck(num_arr, index, size)) num_arr[index] = 0;
				else if (index > 0) index--;	
				break;
			case SEVEN:
			case KEY_SEVEN:
				num_arr[index] = 7;
				if (sameCheck(num_arr, index, size)) num_arr[index] = 0;
				else if (index > 0) index--;	
				break;
			case EIGHT:
			case KEY_EIGHT:
				num_arr[index] = 8;
				if (sameCheck(num_arr, index, size)) num_arr[index] = 0;
				else if (index > 0) index--;	
				break;
			case NINE:
			case KEY_NINE:
				num_arr[index] = 9;
				if (sameCheck(num_arr, index, size)) num_arr[index] = 0;
				else if (index > 0) index--;	
				break;
			case BACK_SPACE:
				num_arr[index] = 0;
				if (index > 0) index--;
				break;
			}

			fndUpdate(dev, num_arr);
		}
		if (!num_arr[0]) {
			index = 0;
		}
		else if (!num_arr[1]) {
			index = 1;
		}
		else if (!num_arr[2]) {
			index = 2;
		}
		else if (!num_arr[3]) {
			index = 3;
		}
		else {
			status = 0;
		}
	}

	return 0;
}

void checkResult(unsigned char *target_arr, unsigned char *answer_arr, int *strike, int *ball){
	*strike = 0;
	*ball = 0;
	int i, j;

	for(i = 1; i < 4; i++){
		for(j = 1; j < 4; j++){
			if(target_arr[i] == answer_arr[j]){
				if(i == j) (*strike)++;
				else (*ball)++;
			}
		}
	}
}

void ledUpdate(int dev, unsigned char *data) {
	ssize_t ret;

	ret = write(dev, data, 1);
	assert2(ret >= 0, "Device write error", LED_DEVICE);
	sleep(1);
}

void ledReset(int dev) {
	unsigned char data = 0;
	ssize_t ret;

	ret = write(dev, &data, 1);
	assert2(ret >= 0, "Device write error", LED_DEVICE);
	sleep(1);
}

int showResult(int dev, int *strike, int *ball){
	unsigned char result = 0;

	switch (*strike) {
	case 4:
		result += 128;
	case 3:
		result += 64;
	case 2:
		result += 32;
	case 1:
		result += 16;
	}

	switch (*ball) {
	case 4:
		result += 8;
	case 3:
		result += 4;
	case 2:
		result += 2;
	case 1:
		result += 1;
	}

	assert(LED_MIN <= result && result <= LED_MAX, "Invalid parameter range");
	ledUpdate(dev, &result);

	if(*strike == 4) return 0;
	return 1;
}

/* Let's test it out */
int main(void) {
	int fnd_dev;
	int led_dev;

	fnd_dev = open(FND_DEVICE, O_RDWR);
	assert2(fnd_dev >= 0, "Device open error", FND_DEVICE);
	led_dev = open(LED_DEVICE, O_RDWR);
	assert2(led_dev >= 0, "Device open error", LED_DEVICE);

	unsigned char target_num[4];
	unsigned char answer_num[4];

	memset(target_num, 0, sizeof(target_num));
	memset(answer_num, 0, sizeof(answer_num));

	unsigned char temp;
	int result = 1;
	int strike, ball;

	if (setNum(fnd_dev, target_num, sizeof(target_num))) return 1;



	while(result){
		if (setNum(fnd_dev, answer_num, sizeof(answer_num))) return 1;
		checkResult(target_num, answer_num, &strike, &ball);
		result = showResult(led_dev, &strike, &ball);

		sleep(3);
		fndReset(fnd_dev);
		ledReset(led_dev);
	}

	close(fnd_dev);
	close(led_dev);
	printf("Program end.\n");

	return 0;
}