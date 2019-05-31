#include "../include/fpga_test.h"

int main(int argc, char **argv) {
	int dev;					// device handler
	unsigned char data[4];		// argv[1] 나누어 담을 char 배열
	ssize_t ret;
	int data_len;
	char *number;
	int i;
	char c;
	char usage[50];
	int stat = 1;

	memset(data, 0, sizeof(data));


	printf("Insert number : ");
	scanf("%s", &number);

	data_len = strlen(number);
	assert2(data_len > 4, "You can only word that unionized less than 4 characters", FND_DEVICE);

	for (i = 0; i < data_len; i++) {
		c = number[i];
		assert('0' <= c && c <= '9', "Invalid digit value");
		data[i] = c - '0';
	}


	dev = open(FND_DEVICE, O_RDWR);
	assert2(dev >= 0, "Device open error", FND_DEVICE);



	while(stat) {
		ret = write(dev, data, FND_MAX_DIGIT);
		assert2(ret >= 0, "Device write error", FND_DEVICE);
		sleep(1);

		memset(data, 0, sizeof(data));
		ret = read(dev, data, FND_MAX_DIGIT);
		assert2(ret >= 0, "Device read error", FND_DEVICE);

		printf("Current FND value: ");
		for (i = 0; i < data_len; i++) {
			printf("%d", data[i]);
		}
		printf("\n");

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