#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/input.h>

//touch screen device path
#define TOUCHSCREEN_DEVICE "/dev/input/event4"
#define EVENT_BUF_NUM 65

int main(void) {

	int dev;
	struct input_event ev[EVENT_BUF_NUM];
	size_t ev_size = sizeof(struct input_event);
	size_t size;

	int i;

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

		size = read(dev, ev, ev_size *EVENT_BUF_NUM);
		if (size < sizeof(struct input_event))
		{
			printf("Device read error : %s\n", TOUCHSCREEN_DEVICE);
			exit(1);
		}

		printf("end read\n");

		for (i = 0; i < (size / sizeof(struct input_event)); i++)
		{
			// 각 event 발생의 type에 따른 분기 처리
			switch (ev[i].type)
			{
			case EV_SYN:
				printf("---------------------------------------n");
				break;
			case EV_KEY:
				printf("Button code %d", ev[i].code);
				switch (ev[i].value)
				{
				case 1:
					printf(": pressedn");
					break;
				case 0:
					printf(": releasedn");
					break;
				default:
					printf("Unknown: type %d, code %d, value %d",
						ev[i].type,
						ev[i].code,
						ev[i].value);
					break;
				}
				break;
			default:
				printf("Unknown: type %d, code %d, value %dn",
					ev[i].type,
					ev[i].code,
					ev[i].value);
				break;
			}
		}
/*












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

		printf("x : %d, y: %d\n", x, y);*/
	}

	close(dev);

	return 0;
}
