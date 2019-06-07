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
	char pnt;

	fd = open(argv[1], O_RDONLY);

	while (1){
		struct input_event ev;

		if (read(fd, &ev, sizeof(struct input_event)) < 0)
		{
			printf("check\n");
			if (errno == EINTR)
				continue;

			break;
		}
		if(ev.value == 1){
			if (ev.type == 1) {
				switch (ev.code) {
				case 2:
					pnt = '1';
					break;
				case 3:
					pnt = '2';
					break;
				case 4:
					pnt = '3';
					break;
				case 5:
					pnt = '4';
					break;
				case 6:
					pnt = '5';
					break;
				case 7:
					pnt = '6';
					break;
				case 8:
					pnt = '7';
					break;
				case 9:
					pnt = '8';
					break;
				case 10:
					pnt = '9';
					break;
				case 11:
					pnt = '0';
					break;
				case 12:
					pnt = '-';
					break;
				case 13:
					pnt = '=';
					break;
				case 14:
					pnt = '\b';
					break;
				case 15:
					pnt = '\t';
					break;
				case 16:
					pnt = 'q';
					break;
				case 17:
					pnt = 'w';
					break;
				case 18:
					pnt = 'e';
					break;
				case 19:
					pnt = 'r';
					break;
				case 20:
					pnt = 't';
					break;
				case 21:
					pnt = 'y';
					break;
				case 22:
					pnt = 'u';
					break;
				case 23:
					pnt = 'i';
					break;
				case 24:
					pnt = 'o';
					break;
				case 25:
					pnt = 'p';
					break;
				case 26:
					pnt = '[';
					break;
				case 27:
					pnt = ']';
					break;
				case 28:
					pnt = '\n';
					break;
				case 30:
					pnt = 'a';
					break;
				case 31:
					pnt = 's';
					break;
				case 32:
					pnt = 'd';
					break;
				case 33:
					pnt = 'f';
					break;
				case 34:
					pnt = 'g';
					break;
				case 35:
					pnt = 'h';
					break;
				case 36:
					pnt = 'j';
					break;
				case 37:
					pnt = 'k';
					break;
				case 38:
					pnt = 'l';
					break;
				case 44:
					pnt = 'z';
					break;
				case 45:
					pnt = 'x';
					break;
				case 46:
					pnt = 'c';
					break;
				case 47:
					pnt = 'v';
					break;
				case 48:
					pnt = 'b';
					break;
				case 49:
					pnt = 'n';
					break;
				case 50:
					pnt = 'm';
					break;
				case 51:
					pnt = ',';
					break;
				case 52:
					pnt = '.';
					break;
				case 53:
					pnt = '/';
					break;
				}

			}

			//printf("%c", pnt);

			printf("text : %c \t\t type : %hu, code : %hu, value : %d\n", pnt, ev.type, ev.code, ev.value);

		}
	}

	close(fd);

	return 0;
}