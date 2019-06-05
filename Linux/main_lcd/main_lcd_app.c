#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/input.h>

//touch screen device path
#define TOUCHSCREEN_DEVICE "/dev/input/event1"

int main(void) {

	int dev;
	struct input_event ev;
	size_t ev_size = sizeof(struct input_event);
	size_t size;

	int x, y;

	//device open
	dev = open(TOUCHSCREEN_DEVICE, O_RDWR);

	//device open error check
	if (dev < 0) {
		printf("Device open error : %s\n", TOUCHSCREEN_DEVICE);
		exit(1);
	}

	printf("open check\n");

	//If you want to expand additional function, Study "#include <linux/input.h>, struct input_event"
	while (1) {
		printf("start read\n");

		size = read(dev, &ev, ev_size);

		printf("end read\n");

		if (size < 0) {
			printf("Touch screen wrong value\n");
			exit(1);
		}

		if (ev.value != 0) {
			x = ev.value;
			size = read(dev, &ev, ev_size);
			if (size < 0) {
				printf("Touch screen wrong value\n");
				exit(1);
			}
			if (ev.value != 0) {
				y = ev.value;
			}
		}

		printf("x : %d, y: %d\n", x, y);
	}

	close(dev);

	return 0;
}
