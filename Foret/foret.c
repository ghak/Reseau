#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFER 1000

int sock;
struct sockaddr_in recv_addr;
struct sockaddr_in exp_addr;
int n, exp_len;

void ouvertureLiaison() {
	sock = socket(PF_INET, SOCK_DGRAM, 0); //creation d'une liaison
	if (sock == -1) {
		perror("socket()");
		exit(1);
	}
	bzero((char *) &recv_addr, sizeof recv_addr); //mise a zero de l'adresse de reception
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	recv_addr.sin_port = htons(5000);
	if (bind(sock, (struct sockaddr *) &recv_addr, sizeof recv_addr) == -1) { //assigne une adresse a sock
		perror("bind()");
		exit(1);
	}
}

char* reception() {
	char* buf = (char*) malloc(BUFFER);
	if ((n = recvfrom(sock, buf, BUFFER, 0, (struct sockaddr *) &exp_addr,
			(socklen_t *) &exp_len)) < 0) {
		perror("recvfrom()");
		exit(1);
	}
	buf[n]='\0';
	return buf;
}

void envoi(char* buf){
	if ((n = sendto(sock, buf, BUFFER, 0, (struct sockaddr *) &exp_addr,
				(socklen_t) exp_len)) < 0) {
			perror("sendto()");
			exit(1);
		}
}

int main() {
	ouvertureLiaison();
	int i = 0;
	while (1) {
		exp_len = sizeof exp_addr;
		char* buf;
		buf=reception();
		i = atoi((const char*)buf);
		i++;
		sprintf(buf, "%d", i);
		envoi(buf);
	}
	close(sock);
	return 0;
}
