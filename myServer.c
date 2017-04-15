#include "unp.h"
struct args{
	long arg1;
	long arg2;
};
struct result{
	long sum;
};
void sum_serv(int sockfd);
void doit(int connfd);
int sockfd_to_family(int sockfd);
void str_echo(int sockfd);
void sig_chld(int signo);
int main(int argc, char **argv){
	int listenfd, connfd;
	pid_t childpid;
	char buff[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in servaddr, cliaddr;
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	Listen(listenfd, LISTENQ);
	Signal(SIGCHLD, sig_chld);
	while(1){
		clilen = sizeof(cliaddr);
		connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);
//		Getsockname(connfd, (SA*)&cliaddr, &len);
//		printf("local addr :%s\n", Sock_ntop((SA*) &cliaddr, len));
//		printf("%d\n", sockfd_to_family(connfd));
		printf("connenction from %s, port %d\n", Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));
		if((childpid = fork()) == 0){
			Close(listenfd);
			sum_serv(connfd);
			exit(0);
		}	
		Close(connfd);
	}
}
void sum_serv(int sockfd){
	struct args args;
	struct result result;
	while(Readn(sockfd, &args, sizeof(args)) > 0){
		result.sum = args.arg1 + args.arg2;
		Writen(sockfd, &result, sizeof(result));
	}
	return;
}

void sig_chld(int signo){
	pid_t pid;
	int stat;
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0){		
		printf("child %d terminated\n", pid);
	}	
	return;
}
void str_echo(int sockfd){
	char buf[MAXLINE];
	ssize_t n;
again:
	while((n = read(sockfd, buf, MAXLINE)) > 0){
		Writen(sockfd, buf, n);
	}
	if(n < 0 && errno == EINTR){
		goto again;
	}else if(n < 0){
		err_sys("read error");
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

