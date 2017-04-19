#include "unp.h"
#include <time.h>
int udp_server(const char *host, const char *serv, socklen_t *addrlenp);
int main(int argc, char** argv){
	socklen_t len;
	time_t ticks;
	int sockfd, n;
	char buf[MAXLINE];
	SA cliaddr;

	if(argc != 2)
		err_quit("usage error");

	sockfd = udp_server(NULL, argv[1], NULL);

	while(1){
		len = sizeof(cliaddr);
		n = Recvfrom(sockfd, buf, MAXLINE, 0, &cliaddr, &len); 
		printf("datagram from %s\n", Sock_ntop(&cliaddr, len));

		ticks = time(NULL);
		snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
		Sendto(sockfd, buf, strlen(buf), 0, &cliaddr, len);
	}
}
int udp_server(const char *host, const char *serv, socklen_t *addrlenp){
	int listenfd, n;
	const int on = 1;
	struct addrinfo hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_DGRAM;

	if((n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("tcp_listen error : unable to getaddrinfo(%s)", gai_strerror(n));
	
	ressave = res;

	do{
		if((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
			continue;
		
		if(bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;

		Close(listenfd);
	}while((res = res->ai_next) != NULL);

	if(res == NULL)
		err_sys("tcp_listen error for %s, %s", host, serv);

	if(addrlenp)
		*addrlenp = res->ai_addrlen;

	freeaddrinfo(ressave);

	return listenfd;
}




