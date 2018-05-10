#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <pthread.h>

#define SERVER_PORT 10007
#define MAX_EVENTS 1024
#define BACKLOG 10

static int stop = 0;
const char *terminate = "close";
size_t      terminatelen = 5;

static void die(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static void setnonblocking(int sock)
{
    int flag = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flag | O_NONBLOCK);
}

static int connect_file(int num)
{
	char file[256];
	struct sockaddr_un sun;
	int sock;
	snprintf(file, sizeof(file), "/tmp/server.%d", num);

	sock = socket(PF_FILE, SOCK_STREAM, 0);
	if (sock == -1) {
		die("socket");
	}
	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, file);

	if (connect(sock, (struct sockaddr*)&sun, sizeof(sun)) == -1){
		die("connect");
	}

	return sock;
}
static int setup_file(int num)
{
	struct sockaddr_un sun;
	int sock;
	char file[256];
	snprintf(file, sizeof(file), "/tmp/server.%d", num);
	if (access(file, F_OK) == 0) {
		unlink(file);
	}
	sock = socket(PF_FILE, SOCK_STREAM, 0);
	if (sock == -1) {
		die("socket");
	}
	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, file);

	if (bind(sock, (struct sockaddr *) &sun, sizeof sun) < 0) {
        close(sock);
        die("bind");
    }

    if (listen(sock, BACKLOG) < 0) {
        close(sock);
        die("listen");
    }

    return sock;
}

static int setup_socket()
{
	int opt = 1;
    int sock;
    struct sockaddr_in sin;

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        die("socket");
    }

    memset(&sin, 0, sizeof sin);
    
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(SERVER_PORT);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sock, (struct sockaddr *) &sin, sizeof sin) < 0) {
        close(sock);
        die("bind");
    }

    if (listen(sock, BACKLOG) < 0) {
        close(sock);
        die("listen");
    }

    return sock;
}

#define cores 3
int servers[cores];

void do_accept()
{
	int i;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
    char buffer[1024];
	off_t off = 0;
	int epfd;
	int listener;

    if ((epfd = epoll_create(MAX_EVENTS)) < 0) {
        die("epoll_create");
    }

    listener = setup_socket();

	for (i=0; i<cores; i++) {
		servers[i] = connect_file(i);
		printf("uds connect #%d: %d\n", i, servers[i]);
	}

    memset(&ev, 0, sizeof ev);
    ev.events = EPOLLIN;
    ev.data.fd = listener;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &ev);

	loff_t in_off = 0, out_off = 0;
	int filedes[2];
	if (pipe(filedes) < 0) {
		die("pipe");
	}

    for (;;) {
		if (stop == 1) {
			break;
		}
        int i;
        int nfd = epoll_wait(epfd, events, MAX_EVENTS, -1);

        for (i = 0; i < nfd; i++) {
            if (events[i].data.fd == listener) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof client_addr;

                int client = accept(listener, (struct sockaddr *) &client_addr, &client_addr_len);
                if (client < 0) {
                    perror("accept");
                    continue;
                }
				int server = servers[client % cores];
				ev.events = EPOLLIN;
				ev.data.fd = client;
				epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);
                //setnonblocking(client);
				printf("accpet: %d <- %d\n", server, client);
            }
			else {
				int client = events[i].data.fd;
				int server = servers[client % cores];

				printf("relay: %d <- %d\n", server, client);
#if 0
				int n = sendfile(server, client, &off, 3);
				if (n <= 0) {
					// TODO: ????
					die("sendfile");
				}
#endif
#if 0
				int err;
				int buf_size = 3;
				err = splice(client, &in_off, filedes[1], NULL, buf_size, SPLICE_F_MOVE | SPLICE_F_MORE);
				if (err < 0) {
					die("splice");
				}
				printf("splice read: %d\n", in_off);
				err = splice(filedes[0], NULL, server, &out_off, buf_size, SPLICE_F_MOVE | SPLICE_F_MORE);
				if (err < 0) {
					die("splice");
				}
				printf("splice send: %d\n", out_off);
#endif
#if 0
				int err;
				int buf_size = 3;
				err = splice(client, NULL, server, NULL, buf_size, SPLICE_F_MOVE | SPLICE_F_MORE);
				if (err < 0) {
					die("splice");
				}
				printf("splice: %ld -> %ld\n", in_off, out_off);
#endif
				int n;
				n = read(client, buffer, sizeof(buffer));
				if (n <= 0) {
					ev.events = EPOLLIN;
					ev.data.fd = client;
					epoll_ctl(epfd, EPOLL_CTL_DEL, client, &ev);
					close(client);
				}
				n = write(server, buffer, sizeof(buffer));
			}
        }
    }
	close(listener);
	close(epfd);
}

int main(int argc, char **argv)
{
	int i;
	do_accept();
	return 0;
}
