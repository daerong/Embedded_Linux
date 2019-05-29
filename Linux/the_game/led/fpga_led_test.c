#include "../include/fpga_test.h"

int main(int argc, char **argv) {
	int dev;					// device handler
	unsigned char data;
	ssize_t ret;
	char usage[50];

	printf("Insert number : ");
	scanf("%d", &data);
	assert(LED_MIN <= data && data <= LED_MAX, "Invalid parameter range");
	
	dev = open(LED_DEVICE, O_RDWR);
	assert2(dev >= 0, "Device open error", LED_DEVICE);

	do {
		ret = write(dev, &data, 1);
		assert2(ret >= 0, "Device write error", LED_DEVICE);
		sleep(500);

		ret = read(dev, &data, 1);
		assert2(ret >= 0, "Device read error", LED_DEVICE);

		printf("Current LED value: %d\n", data);
		printf("\n");

	} while (data--);

	close(dev);
	return 0;
}