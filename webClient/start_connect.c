#include "web.h"
void start_connect(struct file *fptr){
	int fd, flags, n;
	struct addrinfo *ai;

	ai = Host_serv(fptr->f_host, SERV, 0, SOCK_STREAM);
	fd = Socket(ai->ai_family, ai->ai_socktype, ai->protocol);
	fptr->f_fd = fd;
	printf("start_connect for %s, fd %d\n", fptr->f_name, fd);

	flags = Fcntl(fd, F_GETFL, 0);
	Fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	if((n = connect(fd, ai->ai_addr, ai->ai_addrlen)) < 0){
		if(errno != EINPROGRESS)
			err_sys("nonblocking connect error");
		fptr->f_flags = F_CONNECTING;
		FD_SET(&rset, fd);
		FD_SET(&wset, fd);
		maxfd = fd > maxfd ? fd : maxfd;
	}else{
		write_get_cmd(fptr);
	}
}
