#include "../include/fpga_test.h"

int main(int argc, char **argv) {
	int dev;					// device handler
	unsigned char data[4];		// argv[1] 나누어 담을 char 배열
	ssize_t ret;
	int data_len;
	int number;
	int index[4];
	int i;
	char c;
	char usage[50];
	int stat = 1;

	memset(data, 0, sizeof(data));


	printf("Insert number : ");
	scanf("%d", &number);

	if (number >= 1000) data_len = 4;
	else if (number >= 100) data_len = 3;
	else if (number >= 10) data_len = 2;
	else if (number >= 1) data_len = 1;
	else data_len = 0;

	assert2(data_len <= 4, "You can only word that unionized less than 4 characters", FND_DEVICE);

	index[0] = number / 1000;
	number = number - index[0]*1000;
	index[1] = number / 100;
	number = number - index[1]*100;
	index[2] = number / 10;
	number = number - index[2]*10;
	index[3] = number;

	data[3] = index[3];		// 1의 자리
	data[2] = index[2];		// 10의 자리
	data[1] = index[1];		// 100의 자리
	data[0] = index[0];		// 1000의 자리

	dev = open(FND_DEVICE, O_RDWR);
	assert2(dev >= 0, "Device open error", FND_DEVICE);

	while(stat) {
		ret = write(dev, data, FND_MAX_DIGIT);
		assert2(ret >= 0, "Device write error", FND_DEVICE);
		sleep(1);

		if (data[0] != 0) {
			data[0]--;
		}
		else if (data[1] != 0) {
			data[1]--;
			data[0] = 9;
		}
		else if (data[2] != 0) {
			data[2]--;
			data[1] = 9;
			data[0] = 9;
		}
		else if (data[3] != 0) {
			data[3]--;
			data[2] = 9;
			data[1] = 9;
			data[0] = 9;
		}
		else {
			stat = 0;
		}
	}

	close(dev);
	return 0;
}