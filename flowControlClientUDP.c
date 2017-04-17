#include "unp.h"
#define NDG 2000
#define DGLEN 1400
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
	char buf[DGLEN];
	int n;
	for(int i = 0; i < NDG; i++){
		Sendto(sockfd, buf, DGLEN, 0, servaddr, servlen);
	}
}

