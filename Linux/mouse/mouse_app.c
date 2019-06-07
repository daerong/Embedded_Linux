#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>

typedef unsigned int U32;
typedef short U16;

int main(int argc, char** argv) {
	uint8_t keys[128];
	int fd;

	fd = open(argv[1], O_RDONLY);

	while (1) {
		struct input_event ev;

		if (read(fd, &ev, sizeof(struct input_event)) < 0)
		{
			printf("check\n");
			if (errno == EINTR)
				continue;

			break;
		}

		if (ev.type == 1) {
			if (ev.value == 1) {
				if (ev.code == 272) printf("left btn \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);
				else if(ev.code == 273) printf("right btn \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);
			}
		}
		else if (ev.type == 2) {
			if (ev.code == 1) {
				printf("vertical \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);
			}
			else if (ev.code == 0) {
				printf("horizon \t\t type : %hu, code : %hu, value : %d\n", ev.type, ev.code, ev.value);

			}
		}
	}

	close(fd);

	return 0;
}