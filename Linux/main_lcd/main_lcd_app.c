#include "../include/fpga_frame_buffer.h"

int main(void) {

	int touch_dev;
	struct input_event ev;
	size_t ev_size = sizeof(struct input_event);
	size_t size;

	int x, y;

	touch_dev = open(TOUCHSCREEN_DEVICE, O_RDWR);
	assert2(touch_dev >= 0, "Device open error", TOUCHSCREEN_DEVICE);

	//If you want to expand additional function, Study "#include <linux/input.h>, struct input_event"
	while (1) {
		size = read(touch_dev, &ev, ev_size);
		assert(size >= 0, "Touch screen wrong value\n");

		if (ev.value != 0) {
			x = ev.value;
			size = read(touch_dev, &ev, ev_size);
			assert(size >= 0, "Touch screen wrong value\n");
			if (ev.value != 0) {
				y = ev.value;
			}
		}

		printf("x : %d, y: %d\n", x, y);
	}

	close(touch_dev);

	return 0;
}
