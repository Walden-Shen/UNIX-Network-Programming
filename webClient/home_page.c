#include "web.h"
void home_page(const char *host, const char *fname){
	int fd, n;
	char line[MAXLINE];

	fd = Tcp_connect(host, SERV);
	n = snprintf(line, MAXLINE, GET_CMD, fname);
	Writen(fd, line, n);

	while((n = Read(fd, line, MAXLINE)) > 0){
		printf("read %d bytes of the home page\n", n);
	}

	printf("end of file on home page\n");
	close(fd);
}
