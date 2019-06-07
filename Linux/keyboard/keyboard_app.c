#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>

typedef short U16;
typedef int S32;

#define KEYBOARD_DEVICE "/dev/input/event6"

int main(int argc, char** argv) {
	//uint8_t keys[128];
	int fd;
	struct input_event ev;
	/*U16 event_type;
	U16 event_code;
	S32 event_value;*/
	int status;
	char text;
	
	fd = open(KEYBOARD_DEVICE, O_RDONLY);

	while (1) {
		//event_type = 0;
		//event_code = 0;
		//event_value = 0;
		if (read(fd, &ev, sizeof(struct input_event)) < 0)
		{
			printf("check\n");
			if (errno == EINTR)
				continue;

			break;
		}
		// 에러제어 패스
		//event_type = ev.type;
		//event_code = ev.code;
		//event_value = ev.value;
		if (ev.type == 1) {
			switch (ev.code) {
			case 2:
				text = '1';
				break;
			case 3:
				text = '2';
				break;
			case 4:
				text = '3';
				break;
			case 5:
				text = '4';
				break;
			case 6:
				text = '5';
				break;
			case 7:
				text = '6';
				break;
			case 8:
				text = '7';
				break;
			case 9:
				text = '8';
				break;
			case 10:
				text = '9';
				break;
			case 11:
				text = '0';
				break;
			case 12:
				text = '-';
				break;
			case 13:
				text = '=';
				break;
			case 14:
				text = '\b';
				break;
			case 15:
				text = '\t';
				break;
			case 16:
				text = 'q';
				break;
			case 17:
				text = 'w';
				break;
			case 18:
				text = 'e';
				break;
			case 19:
				text = 'r';
				break;
			case 20:
				text = 't';
				break;
			case 21:
				text = 'y';
				break;
			case 22:
				text = 'u';
				break;
			case 23:
				text = 'i';
				break;
			case 24:
				text = 'o';
				break;
			case 25:
				text = 'p';
				break;
			case 26:
				text = '[';
				break;
			case 27:
				text = ']';
				break;
			case 28:
				text = '\n';
				break;
			case 30:
				text = 'a';
				break;
			case 31:
				text = 's';
				break;
			case 32:
				text = 'd';
				break;
			case 33:
				text = 'f';
				break;
			case 34:
				text = 'g';
				break;
			case 35:
				text = 'h';
				break;
			case 36:
				text = 'j';
				break;
			case 37:
				text = 'k';
				break;
			case 38:
				text = 'l';
				break;
			case 44:
				text = 'z';
				break;
			case 45:
				text = 'x';
				break;
			case 46:
				text = 'c';
				break;
			case 47:
				text = 'v';
				break;
			case 48:
				text = 'b';
				break;
			case 49:
				text = 'n';
				break;
			case 50:
				text = 'm';
				break;
			case 51:
				text = ',';
				break;
			case 52:
				text = '.';
				break;
			case 53:
				text = '/';
				break;
			}

			printf("%c", text);
		}
	}

	close(fd);

	return 0;
}