#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "queue.h"
#include "sock.h"
#include "io_link.h"

typedef struct svc_map {
	int svc_port;
	int svc_listen_sock;
	int svc_accpet_sock;
	int proxy_port;
	int proxy_listen_sock;
	int proxy_accept_sock;
} svc_map;

svc_map svcmap = {
	{8800, -1, 18800, -1},
	{8801, -1, 18801, -1},
	{8802, -1, 18802, -1},
	{8803, -1, 18803, -1},
	{8804, -1, 18804, -1},
};

pthread_mutex_t csocks_mutex = PTHREAD_MUTEX_INITIALIZER;
struct Queue* csocks_queue;
extern io_linker sio_linker;


void* worker_thread(void *arg)
{
	int sock;
	int* csock;
	size_t noop = 0;

	printf("new worker thread\n");

retry:
	while (1) {
		pthread_mutex_lock(&csocks_mutex);
		{
			csock = dequeue(csocks_queue);
			if (csock == NULL) {
				pthread_mutex_unlock(&csocks_mutex);
				if (noop++ > 100) {
					usleep(10);
					noop = 0;
				}
				continue;
			}
			sock = *csock;
			free(csock);

			printf("get csock: %d\n", sock);
			noop = 0;
		}
		pthread_mutex_unlock(&csocks_mutex);
		break;
	}

	while (1) {
		// read from client
		char buf[1500];
		size_t bytes = read(sock, buf, sizeof(buf));
		if (bytes <= 0) {
			perror("worker_thread:read\n");
			break;
		}

		// write to iolink node
		if (sio_linker.write_data(&iolink, buf, bytes) < 0) {
			printf("error: sio_linker.write_data\n");
			break;
		}
	}
	goto retry;

	return NULL;
}

#if 1
static int io_sock;
#endif

void* iolink_thread(void *arg)
{
	struct sockaddr_in c_addr;
	socklen_t c_addr_len = sizeof(c_addr);
	struct Queue* iolink_queue;
	io_link iolink;
#if 0
	int io_sock;
#endif
	int *psock;
	int n;

	iolink_queue = newQueue(10);

	// init io linker
	if (sio_linker.init(&iolink) < 0) {
		printf("error: failed to sio_linker init\n");
		return -1;
	}
	printf("sio_linker.init --- OK\n");

	while (1)
	{
		io_sock = accept(iolink.sock, (struct sockaddr*)&c_addr, &c_addr_len);
		n = enqueue(iolink_queue, psock);
		printf("acceot io link node: %d\n", io_sock);
	}
#if 1
	close(io_sock);
#endif
	sio_linker.destroy(&iolink);
	freeQueue(iolink_queue);
}

void* accept_thread(void *arg)
{
	struct sockaddr_in c_addr;
	int s_sock, c_sock;
	int *psock = NULL;
	int n, i;
	socklen_t c_addr_len = sizeof(c_addr);
	struct epoll_event event;
	struct epoll_event *events = NULL;
	int epoll_fd;
	int num_svc_map = 1024;

	// listen svc sock
	for (i=0; i<num_svc_map; i++) {
		int l_sock = tcp_listen_sock(svcmap[i].svc_port);
		svcmap[l_sock].svc_listen_sock = l_sock;
		printf("svc port: %d -> listen sock:%d\n", svcmap[i].svc_port, svcmap[i].svc_listen_sock);
	}
	// create epoll
	epoll_fd = epoll_create(num_svc_map);
	if (epoll_fd < 0) {
		goto done;
	}
	// register event to epoll
	events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * num_svc_map);
	for (i=0; i<num_svc_map; i++) {
		if ((svcmap[i].svc_listen_sock != 0) && (svcmap[i].svc_listen_sock == i)) {
			event.data.fd = svcmap[i].svc_listen_sock;
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, svcmap[i].svc_listen_sock, &event) < 0) {
				goto done;
			}
			printf("epoll add port:%d -> listen sock:%d\n", svcmap[i].svc_port, svcmap[i].svc_listen_sock);
		}
	}

	int res;
	while (1)
	{
		res = epoll_wait(epoll_fd, events, num_svc_map, 100);
		if (res < 0) {
			if (errno == EINTR) continue;
			break;
		}
		else if (res == 0) {
			continue;
		}
		for (i=0; i<res; i++) {
			int l_sock = events[i].data.fd;
			c_sock = accept(svcmap[l_sock].svc_listen_sock, (struct sockaddr*)&c_addr, &c_addr_len);
			printf("accept new client socket: %d, svc port: %d\n", c_sock, svcmap[l_sock].svc_port);

			psock = malloc(sizeof(*psock));
			*psock = c_sock;

			pthread_mutex_lock(&csocks_mutex);
			{
				n = enqueue(csocks_queue, psock);
				printf("enqueue sock:%d in %d\n", *psock, n);
			}
			pthread_mutex_unlock(&csocks_mutex);

		}
	}

done:
	if (events) free(events);
	for (i=0; i<num_svc_map; i++) {
		if ((svcmap[i].svc_listen_sock != 0) && (svcmap[i].svc_listen_sock == i)) {
			close(svcmap[i].svc_listen_sock);
		}
	}
	return NULL;
}

int main(int argc, char **argv)
{
	const static int num_workers = 2;
	struct sockaddr_in c_addr;
	int s_sock, c_sock;
	pthread_t accept_tid;
	pthread_t iolink_tid;
	pthread_t worker_tid[num_workers];
	int i;

	csocks_queue = newQueue(10);

	pthread_create(&accept_tid, NULL, accept_thread, NULL);

	pthread_create(&iolink_tid, NULL, iolink_thread, NULL);

	for (i=0; i<num_workers; i++) {
		pthread_create(&worker_tid[i], NULL, worker_thread, NULL);
	}

	pthread_join(accept_tid, NULL);
	pthread_join(iolink_tid, NULL);
	for (i=0; i<num_workers; i++) {
		pthread_join(worker_tid[i], NULL);
	}

	freeQueue(csocks_queue);

	return 0;
}
