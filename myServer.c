#include "unp.h"
#define OPEN_MAX 100
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
	int listenfd, connfd, i, maxi, sockfd;
	char buff[MAXLINE];
	int nready;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t clilen;
	ssize_t n;
	struct pollfd client[OPEN_MAX];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);
	maxi = 0;
	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	for(i = 1; i < OPEN_MAX; i++){
		client[i].fd = -1;
	}
	while(1){
		nready = Poll(client, maxi + 1, INFTIM);
		if(client[0].revents & POLLRDNORM){			
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);
			printf("connenction from %s, port %d\n", Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));
			for(i = 1; i < OPEN_MAX; i++){
				if(client[i].fd == -1){
					client[i].fd = connfd;
					break;
				}
			}
			if(i == OPEN_MAX){
				err_quit("too many clients");
			}
			client[i].events = POLLRDNORM;
			maxi = i > maxi ? i : maxi;
			if(--nready <= 0){
				continue;
			}
		}
		for(i = 1; i <= maxi; i++){
			if((sockfd = client[i].fd) >= 0){
				if(client[i].revents & (POLLRDNORM | POLLERR)){
					if((n = Read(sockfd, buff, MAXLINE)) == 0 || errno == ECONNRESET){							Close(sockfd);
						client[i].fd = -1;
					}else if(n > 0){
						Writen(sockfd, buff, n);
					}else{
						err_sys("read error");
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

