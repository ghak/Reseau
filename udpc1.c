#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#define BUFFER 1024
//42:2:34:53:10:5:(100:0:9:0:0:1):(19:0:3:20:0:30)

extern int errno;

int main(int argc, char** argv) {
	int sock;
	struct sockaddr_in recv_addr, exp_addr;
	char buf[BUFFER];

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	bzero((char *) &exp_addr, sizeof exp_addr);
	exp_addr.sin_family = AF_INET;
	exp_addr.sin_addr.s_addr = INADDR_ANY;
	exp_addr.sin_port = 0;
	bind(sock, (struct sockaddr *) &exp_addr, sizeof exp_addr);

	bzero((char *) &recv_addr, sizeof recv_addr);
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	recv_addr.sin_port = htons(5000);
	int i = 0;
	int size;
	size = sizeof(struct sockaddr);
	int flag = 1;
	while (flag) {
		bzero(buf, BUFFER);
		int c;
		printf("val?\n");
		scanf("%d",&c);
		gets(buf);
		switch (c) {
		case 1:
			bzero(buf,BUFFER);
			gets(buf);
			sendto(sock, buf, BUFFER, 0, (struct sockaddr *) &recv_addr, size);
			break;
		case 2:
			bzero(buf,BUFFER);
			recvfrom(sock, buf, BUFFER, 0, (struct sockaddr *) &recv_addr,
					&size);
			printf("%s\n", buf);
			break;
		case 3:
			flag = 0;
			break;
		}
	};
	close(sock);
	return 0;
}

