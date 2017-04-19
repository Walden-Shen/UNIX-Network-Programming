#include "unp.h"
#include <time.h>
int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp);
int main(int argc, char** argv){
	socklen_t len;
	time_t ticks;
	int listenfd, connfd;
	char buf[MAXLINE];
	struct sockaddr_storage cliaddr;

	if(argc != 2)
		err_quit("usage error");

	listenfd = tcp_listen(NULL, argv[1], NULL);

	while(1){
		len = sizeof(cliaddr);
		connfd = Accept(listenfd, (SA*)&cliaddr, &len);
		printf("connection from %s\n", sock_ntop((SA*)&cliaddr, len));

		ticks = time(NULL);
		snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
		Write(connfd, buf, strlen(buf));

		Close(connfd);
	}
}
int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp){
	int listenfd, n;
	const int on = 1;
	struct addrinfo hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	if((n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("tcp_listen error : unable to getaddrinfo(%s)", gai_strerror(n));
	
	ressave = res;

	do{
		if((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
			continue;
		Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if(bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;
		Close(listenfd);
	}while((res = res->ai_next) != NULL);

	if(res == NULL)
		err_sys("tcp_listen error for %s, %s", host, serv);
	
	Listen(listenfd, LISTENQ);

	if(addrlenp)
		*addrlenp = res->ai_addrlen;

	freeaddrinfo(ressave);

	return listenfd;
}




