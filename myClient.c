#include "unp.h"
int main(int argc, char** argv){
	char recvline[MAXLINE + 1];
	int sockfd, n;
	struct sockaddr_in servaddr;
	if(argc != 2){
		err_quit("usage error");
	}
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		err_sys("socket error");
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(13);
	if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){
		err_sys("ip address invalid");
	}
	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		err_sys("unable to connect");
	}
	while((n = read(sockfd, recvline, MAXLINE)) > 0){
		recvline[n] = 0;
		if(fputs(recvline, stdout) == EOF){
			err_sys("fputs error");
		}
	}
	if(n < 0){
		err_sys("read error");
	}
	close(sockfd);
}





