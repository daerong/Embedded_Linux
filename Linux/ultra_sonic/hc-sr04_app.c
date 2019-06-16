/* Achro-i.MX6Q External Sensor Test Application
File : hc-sr04_app.c
Auth : gmlee@huins.com */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
int main(void)
{
	int fd;
	int retn;
	int buf = 0;
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
		read(fd, &buf, 4);
		for (loop = 0; loop < 100000; loop++) {};
	}
	close(fd);
	return 0;
}