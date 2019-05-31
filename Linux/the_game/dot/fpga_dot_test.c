#include "../include/fpga_test.h"
#include "../include/fpga_dot_font.h"

int main(int argc, char **argv) {
	int dev, num;
	ssize_t ret;
	int i;
	char usage[50];

	num = 0;

	dev = open(DOT_DEVICE, O_WRONLY);
	assert2(dev >= 0, "Device open error", DOT_DEVICE);

	while () {
		printf("insert num : ");
		scanf("%d", &num);

		ret = write(dev, fpga_number[num], sizeof(fpga_number[num]));
		assert2(ret >= 0, "Device write error", DOT_DEVICE);
		sleep(1);
	}

	close(dev);
	return 0;
}