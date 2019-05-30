#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024
#define ERROR -1

int main(void) {
	int server_socket;
	int client_socket;
	int client_addr_size;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	char buff_receive[BUFF_SIZE + 5];
	char buff_send[BUFF_SIZE + 5];

	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket == ERROR) {
		printf("socket() error : cannot create socket.\n");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(4000);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == ERROR) {
		printf("bind() error : cannot bine socket.\n");
		exit(1);
	}

	while (1) {
		if (listen(server_socket, 5) == ERROR) {
			printf("listen() error : cannot enter listen mode.\n");
			exit(1);
		}

		client_addr_size = sizeof(client_addr);
		client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);

		if (client_socket == ERROR) {
			printf("accept() error : cannot connect client socket.\n");
			exit(1);
		}

		read(client_socket, buff_receive, BUFF_SIZE);
		printf("receive: %s\n", buff_receive);

		sprintf(buff_send, "%d : %s", strlen(buff_receive), buff_receive);
		write(client_socket, buff_send, strlen(buff_send) + 1);          // +1: NULL까지 포함해서 전송
		close(client_socket);
	}
}