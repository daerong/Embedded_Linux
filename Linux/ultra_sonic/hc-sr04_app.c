#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(void)
{
	int fd;
	int retn;
	int buf;
	int loop = 0;
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
		read(fd, &buf, 2);
		usleep(200000);
		printf("distance user : %d (cm)\n", buf);
	}
	close(fd);
	return 0;
}