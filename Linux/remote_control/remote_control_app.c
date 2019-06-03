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
	char buf[4] = { 0 };
	int loop = 0;
	fd = open("/dev/remote_control", O_RDWR);
	printf("fd = %d\n", fd);
	if (fd < 0) {
		perror("/dev/remote_control error");
		exit(-1);
	}
	else {
		printf("< remote control device has been detected >\n");
	}
	while (1) {
		//read(fd, buf, 4);
		//for (loop = 0; loop < 4; loop++) {
		//	printf("%c", buf[loop]);
		//	printf("\n");
		//}
	}
	close(fd);
	return 0;
}