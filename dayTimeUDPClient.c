#include "unp.h"
int udpConnect(const char* host, const char* serv, SA **saptr, socklen_t *lenp);
int main(int argc, char** argv){
	int n, sockfd;
	char recvline[MAXLINE + 1];
	socklen_t salen;
	SA *saptr;

	if(argc != 3)
		err_quit("usage error");

	sockfd = udpConnect(argv[1], argv[2], &saptr, &salen);

	printf("sending to %s\n", Sock_ntop_host(saptr, salen));

	Sendto(sockfd, "", 1, 0, saptr, salen);

	n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
	recvline[n] = 0;
	Fputs(recvline, stdout);

	exit(0);
}
int udpConnect(const char* host, const char* serv, SA **saptr, socklen_t *lenp){
	int sockfd, n;
	struct addrinfo hints, * res, *ressave;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if((n = getaddrinfo(host, serv, &hints, &res)) != 0){
		err_quit("udp_connect error for %s, %s: %s", host, serv, gai_strerror(n));
	}
	ressave = res;

	do{
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(sockfd >= 0)
			break;
	}while((res = res->ai_next) != NULL);

	if(res == NULL)
		err_sys("tcp_connect error for %s, %s", host, serv);

	*saptr = Malloc(res->ai_addrlen);
	memcpy(*saptr, res->ai_addr, res->ai_addrlen);
	*lenp = res->ai_addrlen;

	freeaddrinfo(ressave);
	return(sockfd);
}


			
