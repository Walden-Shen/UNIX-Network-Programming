#include "unp.h"
static int count;
static void recvfrom_int(int signo);
void datagram_echo(int sockfd, SA* cliaddr, socklen_t addrlen);
int main(int argc, char** argv){
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;
	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(sockfd, (SA*) &servaddr, sizeof(servaddr));
	datagram_echo(sockfd,(SA*) &cliaddr, sizeof(cliaddr));

}
void datagram_echo(int sockfd, SA* cliaddr, socklen_t addrlen){
	char buf[MAXLINE];
	socklen_t len;
	Signal(SIGINT, recvfrom_int);
	while(1){
		len = addrlen;
		Recvfrom(sockfd, buf, MAXLINE, 0, cliaddr, &len);
		count++;
	}
}
static void recvfrom_int(int signo){
	printf("%d datagram has been received\n", count);
	exit(0);
}
