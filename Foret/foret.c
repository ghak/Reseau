#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*3636*/

int sock ;
struct sockaddr_in recv_addr ;

int ouvertureLiaison(){
	sock = socket (PF_INET, SOCK_DGRAM, 0) ;//creation d'une liaison
	bzero ((char *) &recv_addr, sizeof recv_addr) ;//mise a zero de l'adresse de reception
	recv_addr.sin_family = AF_INET ;
	recv_addr.sin_addr.s_addr = htonl (INADDR_ANY) ;
	recv_addr.sin_port = htons (5000) ;
	bind (sock, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) ;//assigne une adresse a sock 
}

int main(){
	
}
