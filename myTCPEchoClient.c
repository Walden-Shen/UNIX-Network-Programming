#include "unpthread.h"
#include <time.h>
struct args{
	long arg1;
	long arg2;
};
struct result{
	long sum;
};
static int staticsockfd;
static FILE *staticfp;
void str_cli_thread(FILE *fp_arg, int sockfd_arg);
void* copyto(void *arg);
void forkNonblock(FILE *fp, int sockfd);
void str_cli_select(FILE* fp, int sockfd);
void str_cli(FILE* fp, int sockfd);
void sum_cli(FILE* fp, int sockfd);
void nonblock_strcli(FILE *fp, int sockfd);
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
	str_cli_thread(stdin, sockfd);
	exit(0);
}
void str_cli_thread(FILE *fp_arg, int sockfd_arg){
	char recvline[MAXLINE];
	pthread_t tid;
	staticsockfd = sockfd_arg;
	staticfp = fp_arg;

	Pthread_create(&tid, NULL, copyto, NULL);

	while(Readline(staticsockfd, recvline, MAXLINE) > 0){
		Fputs(recvline, stdout);
	}
}
void *copyto(void *arg){
	char sendline[MAXLINE];
	while(Fgets(sendline, MAXLINE, staticfp) != NULL){
		Writen(staticsockfd, sendline, strlen(sendline));
	}
	Shutdown(staticsockfd, SHUT_WR);
	return (NULL);
}

void forkNonblock(FILE *fp, int sockfd){
	pid_t pid;
	char sendline[MAXLINE], recvline[MAXLINE + 1];

	if((pid = fork()) == 0){
		while(readline(sockfd, recvline, MAXLINE) > 0){
			Writen(fileno(stdout), recvline, strlen(recvline));
		}
		kill(getppid(), SIGTERM);
	}
	
	while(Fgets(sendline, MAXLINE, fp) != NULL)
		Writen(sockfd, sendline, strlen(sendline));
	Shutdown(sockfd, SHUT_WR);
	pause();
	return;
}
void nonblock_strcli(FILE *fp, int sockfd){
	int maxfdp1, stdineof = 0, val;
	char to[MAXLINE], fr[MAXLINE];
	char *froptr, *friptr, *tooptr, *toiptr;
	fd_set rset, wset;
	ssize_t n, nwritten;

	val = Fcntl(sockfd, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

	val = Fcntl(STDIN_FILENO, F_GETFL, 0);
	Fcntl(STDIN_FILENO, F_SETFL, val | O_NONBLOCK);

	val = Fcntl(STDOUT_FILENO, F_GETFL, 0);
	Fcntl(STDOUT_FILENO, F_SETFL, val | O_NONBLOCK);

	froptr = friptr = &fr[0];
	tooptr = toiptr = &to[0];

	maxfdp1 = max(max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;

	while(1){
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		if(stdineof != 1 && toiptr < &to[MAXLINE]){
			FD_SET(STDIN_FILENO, &rset);
		}
		if(toiptr < &fr[MAXLINE]){
			FD_SET(sockfd, &rset);
		}
		if(toiptr > tooptr){
			FD_SET(sockfd, &wset);
		}
		if(friptr > froptr){
			FD_SET(STDOUT_FILENO, &wset);
		}
		Select(maxfdp1, &rset, &wset, NULL, NULL);
		if(FD_ISSET(STDIN_FILENO, &rset)){
			if((n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0){
				if(errno != EWOULDBLOCK){
					err_sys("read error on stdin");
				}
			}else if(n == 0){
				stdineof = 1;
				fprintf(stderr, "%s: EOF on stdin\n", gf_time());
				if(toiptr = tooptr){
					Shutdown(sockfd, SHUT_WR);
				}
			}else{
				fprintf(stdin, "%s: read %d bytes from stdin\n", gf_time(), n);
				toiptr += n;
				FD_SET(sockfd, &wset);
			}
		}
		if(FD_ISSET(sockfd, &rset)){
			if((n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0){
				if(errno != EWOULDBLOCK){
					err_sys("read error on sockfd");
				}
			}else if(n == 0){
				fprintf(stderr, "%s: EOF on socket\n", gf_time());
				if(stdineof == 0)
					err_quit("the server terminate prematurely");
				else
					return;
			}else{
				fprintf(stderr, "%s: %d bytes read from socket\n", gf_time(), n);
				friptr += n;
				FD_SET(STDOUT_FILENO, &wset);
			}
		}
		if(FD_ISSET(STDOUT_FILENO, &wset) && (n = friptr - froptr) > 0){
			if((nwritten = write(STDOUT_FILENO, froptr, n)) < 0){
				if(errno != EWOULDBLOCK){
					err_sys("read error on stdout");
				}
			}else{
				fprintf(stderr, "%s: write %d bytes to stdout\n", gf_time(), nwritten);
				froptr += n;
				if(froptr == friptr)
					froptr = friptr = fr;
			}
		}
		if(FD_ISSET(sockfd, &wset) && (n = toiptr - tooptr) > 0){
			if((nwritten = write(sockfd, tooptr, n)) < 0){
				if(errno != EWOULDBLOCK){
					err_sys("read error on stdout");
				}
			}else{
				fprintf(stderr, "%s: write %d bytes to socket\n", gf_time(), nwritten);
				tooptr += n;
				if(tooptr == toiptr)
					tooptr = toiptr = to;
					if(stdineof == 1)
						Shutdown(sockfd, SHUT_WR);
			}
		}
	}
}
char* gf_time(){
	struct timeval tv;
	static char str[30];
	char *ptr;
	if(gettimeofday(&tv, NULL) < 0)
		err_sys("gettimeofday error");
	ptr = ctime(&tv.tv_sec);
	strcpy(str, &ptr[11]);
	snprintf(str + 8, sizeof(str) - 8, ".%06ld", tv.tv_usec);
	return str;
}

void str_cli_select(FILE* fp, int sockfd){
	int maxfdp1, stdineof = 0, n;
	char buf[MAXLINE];
	fd_set rset;
	FD_ZERO(&rset);
	while(1){
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		if(stdineof == 0){
			FD_SET(fileno(fp), &rset);
		}
		FD_SET(sockfd, &rset);
		Select(maxfdp1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(fileno(fp), &rset)){
			if((n = Read(fileno(fp), buf, MAXLINE)) != 0){
				Writen(sockfd, buf, n);
			}else{
				Shutdown(sockfd, SHUT_WR);
				stdineof = 1;
				FD_CLR(fileno(fp), &rset);
				continue;
			}
		}
		if(FD_ISSET(sockfd, &rset)){
			if((n = Read(sockfd, buf, MAXLINE)) == 0){
				if(stdineof == 0){
					err_quit("server terminate prematurely");
				}else{
					return;
				}
			}
			Fputs(buf, stdout);
		}
	}
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
	




