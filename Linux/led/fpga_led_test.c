#include "../include/fpga_test.h"

int main(int argc, char **argv) {
	int dev;					// device handler
	unsigned char data;
	ssize_t ret;
	char usage[50];

	sprintf(usage, "Usage:\n\tfpga_led_test <%d ~ %d>\n", LEDS_MIN, LEDS_MAX);
	assert(argc == 2, usage);

	data = atoi(argv[1]);
	assert(LEDS_MIN <= data && data <= LEDS_MAX, "Invalid parameter range");
	
	dev = open(LEDS_DEVICE, O_RDWR);
	assert2(dev >= 0, "Device open error", LEDS_DEVICE);

	ret = write(dev, &data, 1);
	assert2(ret >= 0, "Device write error", LEDS_DEVICE);
	sleep(1);

	ret = read(dev, &data, 1);
	assert2(ret >= 0, "Device read error", LEDS_DEVICE);

	printf("Current LED value: %d\n", data);
	printf("\n");

	close(dev);
	return 0;
}