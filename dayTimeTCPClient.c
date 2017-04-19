#include "unp.h"
int tcpConnect(const char* host, const char* serv);
int main(int argc, char** argv){
	int n, sockfd;
	char recvline[MAXLINE + 1];
	socklen_t len;
	struct sockaddr_storage ss;

	if(argc != 3)
		err_quit("usage error");

	sockfd = tcpConnect(argv[1], argv[2]);
	len = sizeof(ss);
	Getpeername(sockfd, (SA*)&ss, &len);
	printf("connected to %s\n", Sock_ntop_host((SA*)&ss, len));

	while((n = Read(sockfd, recvline, MAXLINE)) > 0){
		recvline[n] = 0;
		Writen(fileno(stdout), recvline, strlen(recvline));
	}
	exit(0);
}
int tcpConnect(const char* host, const char* serv){
	int sockfd, n;
	struct addrinfo hints, * res, *ressave;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if((n = getaddrinfo(host, serv, &hints, &res)) != 0){
		err_quit("tcp_connect error for %s, %s: %s", host, serv, gai_strerror(n));
	}
	ressave = res;
	do{
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(sockfd < 0)
			continue;
		struct sockaddr* sockaddr = res->ai_addr;
		if(connect(sockfd, sockaddr, res->ai_addrlen) == 0){
			break;
		}
		Close(sockfd);
	}while((res = res->ai_next) != NULL);

	if(res == NULL){
		err_sys("tcp_connect error for %s, %s", host, serv);
	}
	
	freeaddrinfo(ressave);
	return(sockfd);
}


			
