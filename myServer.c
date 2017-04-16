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
	int listenfd, connfd, i, maxi, maxfd, sockfd;
	char buff[MAXLINE];
	int nready, client[FD_SETSIZE];
	struct sockaddr_in servaddr, cliaddr;
	fd_set rset, allset;
	socklen_t clilen;
	ssize_t n;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);
	maxfd = listenfd;
	maxi = -1;
	for(i = 0; i < FD_SETSIZE; i++){
		client[i] = -1;
	}
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	while(1){
		rset = allset;
		nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(listenfd, &rset)){			
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);
			printf("connenction from %s, port %d\n", Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));
			for(i = 0; i < FD_SETSIZE; i++){
				if(client[i] == -1){
					client[i] = connfd;
					break;
				}
			}
			if(i == FD_SETSIZE){
				err_quit("too many clients");
			}	
			FD_SET(connfd, &allset);
			maxfd = connfd > maxfd ? connfd : maxfd;
			maxi = i > maxi ? i : maxi;
			if(--nready <= 0){
				continue;
			}
		}
		for(i = 0; i <= maxi; i++){
			if((sockfd = client[i]) >= 0){
				if(FD_ISSET(sockfd, &rset)){
					if((n = Read(sockfd, buff, MAXLINE)) == 0){
						Close(sockfd);
						FD_CLR(sockfd, &allset);
						client[i] = -1;
					}else{
						Writen(sockfd, buff, n);
					}
					if(--nready <= 0){
						break;
					}
				}
			}
		}
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

