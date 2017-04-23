#include "web.h"
int main(int argc, char **argv){
	int i, fd, n, maxnconn, flags, error;
	char buf[MAXLINE];
	fd_set rs, ws;

	if(argc < 5)
		err_quit("usage: web <#conns> <hostname> <homepage> <file1> ...");

	maxnconn = atoi(argv[1]);
	nfiles = min(MAXFILES, argc - 4);

	for(i = 0; i < nfiles; i++){
		file[i].f_name = argv[i + 4];
		file[i].f_host = argv[2];
		file[i].f_flags = 0;
	}
	printf("nfiles = %d\n", nfiles);

	home_page(argv[2], argv[3]);

	FD_ZERO(&rs);
	FD_ZERO(&ws);
	maxfd = -1;
	nlefttoread = nlefttoconn = nfiles;
	nconn = 0;

	while(nlefttoread > 0){	
		while(nconn <= maxnconn){
			for(i = 0; i < nfiles; i++){
				if(file[i].f_flags == 0){
					break;
				}
			}
			if(i == nfiles)
				err_quit("nlefttoread = %d, but nothing found", nlefttoread);
			start_connect(&file[i]);
			nconn++;	
			nlefttoconn--;
		}
		rs = rset;
		ws = wset;
		n = Select(maxfd + 1, &rs, &ws, NULL, NULL);
		for(i = 0; i< nfiles; i++){
			if(file[i].f_flags == F_CONNECTING && (FD_ISSET(file[i].f_fd, &rs) || FD_ISSET(file[i].f_fd, &ws))){
				n = sizeof(error);
				if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &n) < 0 || error != 0)
					err_ret("nonblocking connection failed for %s", file[i].f_name);
				printf("connection established for %s\n", file[i].f_name);
				FD_CLR(fd, &wset);
				write_get_cmd(&file[i]);
			}else if(file[i].f_flags == F_READING && FD_ISSET(file[i].f_fd, &ws)){
				if((n = Read(file[i].f_fd, buf, MAXLINE)) > 0){
					printf("read %d bytes from %s\n", n, file[i].f_name);
				}else{
					file[i].f_flags == F_DONE;
					FD_CLR(file[i].f_fd, &rset);
					Close(file[i].f_fd);
					nconn--;
					nlefttoread--;
				}
			}
		}
	}
	exit(0);
}


