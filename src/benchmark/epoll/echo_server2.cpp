// 출처: http://d.hatena.ne.jp/odz/20070507/1178558340
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <pthread.h>
#include "librte_ring/rte_ring.h"

#define SERVER_PORT 10007
#define MAX_EVENTS 10
#define BACKLOG 10

static struct rte_ring *ring = NULL;
static int stop = 0;
static int listener;
static int epfd;
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

void do_accept()
{
	int i;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
    char buffer[1024];

    if ((epfd = epoll_create(MAX_EVENTS)) < 0) {
        die("epoll_create");
    }

    listener = setup_socket();

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

                setnonblocking(client);
				rte_ring_sp_enqueue(ring, &client);
            }
        }
    }
}

void* handler(void *arg)
{
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
    char buffer[1024];
	int n;
	int i;
	int client;
	void *burst[4] = {0};
	int timeout = 100;

    if ((epfd = epoll_create(MAX_EVENTS)) < 0) {
        die("epoll_create");
    }

    for (;;) {
		if (stop == 1) {
			break;
		}

		n = rte_ring_mc_dequeue_burst(ring, burst, 4, NULL);
		for (i=0; i<n; i++) {
			memset(&ev, 0, sizeof ev);
			ev.events = EPOLLIN | EPOLLET;
			client = ((int*)burst)[i];
			ev.data.fd = client;
			epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);
		}

        int nfd = epoll_wait(epfd, events, MAX_EVENTS, timeout);
        for (i = 0; i < nfd; i++) {
			int client = events[i].data.fd;
			int n = read(client, buffer, sizeof buffer);
			if (n < 0) {
				perror("read");
				epoll_ctl(epfd, EPOLL_CTL_DEL, client, &ev);
				close(client);
			} else if (n == 0) {
				epoll_ctl(epfd, EPOLL_CTL_DEL, client, &ev);
				close(client);
			} else {
				if (memcmp(terminate, buffer, terminatelen) == 0) {
					stop = 1;
					break;
				}
				write(client, buffer, n);
			}
		}
    }
    return NULL;
}

#define RING_NAME "rte_ring"
#define RING_SIZE 4096

int main(int argc, char **argv)
{
	int i;
	int N = atoi(argv[1]);
	pthread_t tid[N];

	ring = rte_ring_create(RING_NAME, RING_SIZE, rte_socket_id(), RING_F_SP_ENQ);

	for (i=0; i<N; i++) {
		pthread_create(&tid[i], NULL, handler, NULL);
	}

	do_accept();

	for (i=0; i<N; i++) {
		pthread_join(tid[i], NULL);
	}
	close(listener);
	return 0;
}
