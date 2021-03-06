#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#define BUFFER 80 
extern int errno ;

int main (){
  int sock ;
  struct sockaddr_in recv_addr, exp_addr ;
  char buf [BUFFER] ;

  sock = socket (PF_INET, SOCK_DGRAM, 0) ;
  bzero ((char *) &exp_addr, sizeof exp_addr) ;
  exp_addr.sin_family = AF_INET ;
  exp_addr.sin_addr.s_addr = INADDR_ANY ;
  exp_addr.sin_port = 0 ;
  bind (sock, (struct sockaddr *)&exp_addr, sizeof exp_addr) ;

  bzero ((char *) &recv_addr, sizeof recv_addr) ;
  recv_addr.sin_family = AF_INET ;
  recv_addr.sin_addr.s_addr = inet_addr ("127.0.0.1") ;
  recv_addr.sin_port = htons (6000) ;
  
  strcpy (buf, "init") ;
  sendto (sock, buf, 24, 0, (struct sockaddr *)&recv_addr, sizeof (struct sockaddr)) ;
  int size = sizeof (struct sockaddr);
	while(1){
		recvfrom(sock, buf, BUFFER, 0, (struct sockaddr *) &recv_addr,	&size);
		sleep(1);
		if(strstr(buf,"WARNING")){
			perror("Warning");
			exit(1);
		} 
		int a;
		a=atoi(buf);
		printf("%d\n",a);
		a++;
		sprintf(buf,"%d",a);
		sendto (sock, buf, 24, 0, (struct sockaddr *)&recv_addr, sizeof (struct sockaddr)) ;
	}
  close (sock) ;
  return 0 ;
}

