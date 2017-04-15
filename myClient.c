#include "unp.h"
struct args{
	long arg1;
	long arg2;
};
struct result{
	long sum;
};
void str_cli(FILE* fp, int sockfd);
void sum_cli(FILE* fp, int sockfd);
int main(int argc, char** argv){
	char recvline[MAXLINE + 1];
	int sockfd, n;
	struct sockaddr_in servaddr;
	if(argc != 2){
		err_quit("usage error");
	}
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	Connect(sockfd, (SA*) &servaddr, sizeof(servaddr));
	sum_cli(stdin, sockfd);
	exit(0);
}
void sum_cli(FILE* fp, int sockfd){
	char sendline[MAXLINE];
	struct args args;
	struct result result;
	while(Fgets(sendline, MAXLINE, fp) != NULL){
		if(sscanf(sendline, "%ld%ld", &args.arg1, &args.arg2) != 2){
			Fputs("error with input", stdout);
			continue;
		}
		Writen(sockfd, &args, sizeof(args));
		if(Readn(sockfd, &result, sizeof(result)) == 0){
			err_quit("terminate prematurely");
		}
		printf("%ld\n", result.sum);
	}
	return;
}


void str_cli(FILE* fp, int sockfd){
	char sendline[MAXLINE], recvline[MAXLINE];
	while(Fgets(sendline, MAXLINE, fp) != NULL){
		Writen(sockfd, sendline, strlen(sendline));
		if(Readline(sockfd, recvline, MAXLINE) == 0){
			err_quit("server terminate prematurely");
		}
		Fputs(recvline, stdout);
	}
}
	




