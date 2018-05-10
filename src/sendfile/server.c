#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <pthread.h>

#define SERVER_PORT 11100
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

static int setup_socket(int port)
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
    sin.sin_port = htons(port);

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

static int num_client = 0;

void* do_accept(void *arg)
{
	int listener;
	int i;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
    char buffer[1024];
	int epfd;

    listener = setup_file(num_client++);
	printf("server: %d\n", listener);

    if ((epfd = epoll_create(MAX_EVENTS)) < 0) {
        die("epoll_create");
    }
	printf("server: %d, epoll: %d\n", listener, epfd);

    memset(&ev, 0, sizeof ev);
    ev.events = EPOLLIN;
    ev.data.fd = listener;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &ev);

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
				printf("server: %d, accept: %d\n", listener, client);
				ev.events = EPOLLIN;
				ev.data.fd = client;
				epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);
                //setnonblocking(client);
            }
			else {
				int client = events[i].data.fd;
				memset(buffer, 0, sizeof(buffer));
				int n = read(client, buffer, sizeof(buffer));
				if (n <= 0) {
					ev.events = EPOLLIN;
					ev.data.fd = client;
					epoll_ctl(epfd, EPOLL_CTL_DEL, client, &ev);
					printf("server: %d, %d bye~\n", listener, client);
					close(client);
				}
				else{
					printf("%d: %d: %s\n", listener, client, buffer);
				}
			}
        }
    }
	close(listener);
	close(epfd);
}

#define NUM 3
int main(int argc, char **argv)
{
	int i;
	pthread_t tid[NUM];
	for (i=0; i<NUM; i++) {
		pthread_create(&tid[i], NULL, do_accept, NULL);
		usleep(10);
	}
	for (i=0; i<NUM; i++) {
		pthread_join(tid[i], NULL);
	}
	return 0;
}
