#include "unp.h"
void datagram_cli(FILE* fp, int sockfd, SA* servaddr, socklen_t servlen);
int main(int argc, char** argv){
	if(argc != 2){
		err_quit("usage error\n");
	}
	int sockfd;
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	datagram_cli(stdin, sockfd, (SA*) &servaddr, sizeof(servaddr));
}
void datagram_cli(FILE* fp, int sockfd, SA* servaddr, socklen_t servlen){
	char buf[MAXLINE];
	int n;

	while(Fgets(buf, MAXLINE, fp) != NULL){
		Sendto(sockfd, buf, strlen(buf), 0, servaddr, servlen);

		n = Recvfrom(sockfd, buf, MAXLINE, 0, NULL, NULL);
		buf[n] = '\0';

		Writen(fileno(stdout), buf, n);
	}
}

