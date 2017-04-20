#include "unp.h"
void datagram_cli(FILE* fp, int sockfd, SA* servaddr, socklen_t servlen);
static void sig_alarm(int signo);
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

	Connect(sockfd, servaddr, servlen);

	while(Fgets(buf, MAXLINE, fp) != NULL){
		Sendto(sockfd, buf, strlen(buf), 0, servaddr, servlen);

		alarm(5);

		if((n = recvfrom(sockfd, buf, MAXLINE, 0, NULL, NULL)) < 0){
			if(errno = EINTR){
				fprintf(stderr, "socket timeout\n");
			}else{
				err_sys("recvfrom error");
			}
		}else{
			alarm(0);
			buf[n] = '\0';
			Writen(fileno(stdout), buf, n);
		}
	}
}
static void sig_alarm(int signo){
	return;
}

