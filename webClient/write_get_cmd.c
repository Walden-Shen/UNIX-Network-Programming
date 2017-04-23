#include "web.h"
void write_get_cmd(struct file *fptr){
	char line[MAXLINE];

	snprintf(line, MAXLINE, GET_CMD, fptr->f_name);
	Writen(fptr->f_fd, line, strlen(line));
	printf("wrote %d bytes for %s\n", strlen(line), fptr->f_name);

	fptr->f_flags = F_READING;

	FD_SET(&rset, fptr->f_fd);

	maxfd = maxfd > fptr->f_fd ? maxfd : fptr->f_fd;
}
