#include "unp.h"
int main(int argc, char** argv){
	int n, sockfd;
	char recvline[MAXLINE + 1];
	struct sockaddr_in servaddr;
	struct in_addr** pptr;
	struct in_addr* inetaddrp[2];
	struct in_addr inetaddr;
	struct hostent* hp;
	struct servent* sp;

	if(argc != 3)
		err_quit("usage error");

	if((sp = getservbyname(argv[2], NULL)) == NULL)
		err_quit("service doesn't exist");

	if((hp = gethostbyname(argv[1])) == NULL){
		if(inet_aton(argv[1], &inetaddr) == 0){
			err_quit("unable to resolve the ip address");
		}else{
			inetaddrp[0] = &inetaddr;
			inetaddrp[1] = NULL;
			pptr = inetaddrp;
		}
	}else{
		pptr = (struct in_addr**)hp->h_addr_list;
	}

	for(; *pptr != NULL; pptr++){
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = sp->s_port;
		servaddr.sin_addr = **pptr;
		sockfd = Socket(AF_INET, SOCK_STREAM, 0);
		printf("trying to connect to %s\n",inet_ntoa(**pptr));
		if(connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) == 0)
			break;
		err_ret("connect error");
		Close(sockfd);
	}
	if(*pptr == NULL)
		err_quit("unable to connect");
	while((n = Read(sockfd, recvline, MAXLINE)) > 0){
		recvline[n] = 0;
		Writen(fileno(stdout), recvline, strlen(recvline));
	}
	exit(0);
}
