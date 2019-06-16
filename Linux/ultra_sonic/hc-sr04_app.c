//mknod /dev/us c 245 0

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(void)
{
	int fd;
	int retn;
	int *buf = (int *)malloc(sizeof(int));
	int loop = 0;
	char state = 1;
	fd = open("/dev/us", O_RDWR);
	printf("fd = %d\n", fd);
	if (fd < 0) {
		perror("/dev/us error");
		exit(-1);
	}
	else {
		printf("< us device has been detected >\n");
	}
	while (1) {
		state = read(fd, buf, 4);
		printf("distance user : %d (cm)\n", *buf);
		usleep(200000);
	}
	close(fd);
	return 0;
}