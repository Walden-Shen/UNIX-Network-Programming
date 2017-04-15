#include "unp.h"
void doit(int connfd);
int sockfd_to_family(int sockfd);
int main(int argc, char **argv){
	int listenfd, connfd;
	char buff[MAXLINE];
	socklen_t len;
	struct sockaddr_in servaddr, cliaddr;
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_port = htons(13);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	Listen(listenfd, LISTENQ);
	while(1){
		len = sizeof(cliaddr);
		connfd = Accept(listenfd, (SA*)&cliaddr, &len);
		printf("%d\n", sockfd_to_family(connfd));
		printf("connenction from %s, port %d\n", Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));
		if(fork() == 0){
			Close(listenfd);
			doit(connfd);
			Close(connfd);
			exit(0);
		}
		Close(connfd);
	}
}
void doit(int connfd){
	char buff[MAXLINE] = "test for once\n";
	write(connfd, buff, strlen(buff));
	return;
}
int sockfd_to_family(int sockfd){
	struct sockaddr_storage ss;
	socklen_t len;
	len = sizeof(ss);
	if(getsockname(sockfd, (SA*)&ss, &len) != 0){
		return (-1);
	}
	return ss.ss_family;
}

