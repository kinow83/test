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

typedef struct svc_portmap_t {
	int svc_port;
	int proxy_port;
	int svc_accpet_sock;
	int proxy_accept_sock;
} svc_portmap_t;

svc_portmap_t svc_port_defined = {
	{8800, 18800, -1, -1, -1, -1},
	{8801, 18801, -1, -1, -1, -1},
	{8802, 18802, -1, -1, -1, -1},
	{8803, 18803, -1, -1, -1, -1},
	{8804, 18804, -1, -1, -1, -1},
};

#define NUM_SVC_PORT 1024
svc_portmap_t svc_portmap[NUM_SVC_PORT];

pthread_mutex_t svc_lock = PTHREAD_MUTEX_INITIALIZER;
struct Queue* svcsock_queue;
extern iolinker io_offerer;


void* svc_worker_thread(void *arg)
{
	int sock;
	int* psock;
	size_t noop = 0;
	ioindex_t ioindex;

	printf("run svc worker thread\n");

retry:
	while (1) {
		pthread_mutex_lock(&svc_lock);
		{
			psock = dequeue(svcsock_queue);
			if (psock == NULL) {
				pthread_mutex_unlock(&svc_lock);
				if (noop++ > 100) {
					usleep(10);
					noop = 0;
				}
				continue;
			}
			sock = *psock;
			noop = 0;
		}
		pthread_mutex_unlock(&svc_lock);
		free(psock);
		printf("svc_worker_thread:get sock: %d\n", sock);
		break;
	}

	while (1) {
		// read from client
		char buf[1500];
		size_t bytes = read(sock, buf, sizeof(buf));
		if (bytes <= 0) {
			perror("svc_worker_thread:read\n");
			break;
		}

		if (io_offerer.put(&offerer, buf, bytes, &ioindex)) {
			printf("error: svc_worker_thread:put\n");
			break;
		}
		if (offerer.send(&iolink, buf, bytes) < 0) {
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

void* iolink_accept_thread(void *arg)
{
	struct sockaddr_in caddr;
	socklen_t caddr_len = sizeof(caddr);

	if (io_offerer.init(&iolink) < 0) {
		printf("error: failed to sio_linker init\n");
		return -1;
	}
	printf("sio_linker.init --- OK\n");

	while (1)
	{
		io_sock = accept(iolink.sock, (struct sockaddr*)&caddr, &caddr_len);
		n = enqueue(iolink_queue, psock);
		printf("acceot io link node: %d\n", io_sock);
	}
#if 1
	close(io_sock);
#endif
	sio_linker.destroy(&iolink);
	freeQueue(iolink_queue);
}

void* svc_accept_thread(void *arg)
{
	struct sockaddr_in caddr;
	int ssock, csock;
	int *psock = NULL;
	int n, i;
	socklen_t caddr_len = sizeof(caddr);
	struct epoll_event event;
	struct epoll_event *events = NULL;
	int epoll_fd;
	int num_svc_port = sizeof(svc_port_defined)/sizeof(svc_portmap_t);

	// listen svc port
	for (i=0; i<num_svc_port; i++) {
		int lport = svc_port_defined[i].svc_port;
		int lsock = tcp_listen_sock(lport);
		svc_portmap[lsock].svc_listen_sock = lsock;
		printf("svc port: %d -> listen sock:%d\n", svc_portmap[i].svc_port, svc_portmap[i].svc_listen_sock);
	}

	// create epoll
	epoll_fd = epoll_create(num_svc_port);
	if (epoll_fd < 0) {
		goto done;
	}
	// register event to epoll
	events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * num_svc_port);
	for (i=0; i<num_svc_port; i++) {
		if ((svc_portmap[i].svc_listen_sock != 0) && (svc_portmap[i].svc_listen_sock == i)) {
			event.data.fd = svc_portmap[i].svc_listen_sock;
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, svc_portmap[i].svc_listen_sock, &event) < 0) {
				goto done;
			}
			printf("epoll add port:%d -> listen sock:%d\n", svc_portmap[i].svc_port, svc_portmap[i].svc_listen_sock);
		}
	}

	int res;
	while (1)
	{
		res = epoll_wait(epoll_fd, events, num_svc_port, 100);
		if (res < 0) {
			if (errno == EINTR) continue;
			break;
		}
		else if (res == 0) {
			continue;
		}
		for (i=0; i<res; i++) {
			int lsock = events[i].data.fd;
			csock = accept(svc_portmap[lsock].svc_listen_sock, (struct sockaddr*)&caddr, &caddr_len);
			printf("accept new client socket: %d, svc port: %d\n", csock, svc_portmap[lsock].svc_port);

			psock = malloc(sizeof(*psock));
			*psock = csock;

			pthread_mutex_lock(&svc_lock);
			{
				n = enqueue(svcsock_queue, psock);
				printf("enqueue sock:%d in %d\n", *psock, n);
			}
			pthread_mutex_unlock(&svc_lock);

		}
	}

done:
	if (events) free(events);
	for (i=0; i<num_svc_port; i++) {
		if ((svc_portmap[i].svc_listen_sock != 0) && (svc_portmap[i].svc_listen_sock == i)) {
			close(svc_portmap[i].svc_listen_sock);
		}
	}
	return NULL;
}

int main(int argc, char **argv)
{
	const static int num_workers = 2;
	struct sockaddr_in caddr;
	int ssock, csock;
	pthread_t svc_accept_tid;
	pthread_t svc_worker_tid[num_workers];
	pthread_t iolink_tid;
	int i;

	svcsock_queue = newQueue(10);

	// svc threads
	pthread_create(&svc_accept_tid, NULL, svc_accept_thread, NULL);
	for (i=0; i<num_workers; i++) {
		pthread_create(&svc_worker_tid[i], NULL, svc_worker_thread, NULL);
	}

	// io link threads
	pthread_create(&iolink_tid, NULL, iolink_accept_thread, NULL);

	pthread_join(svc_accept_tid, NULL);
	pthread_join(iolink_tid, NULL);
	for (i=0; i<num_workers; i++) {
		pthread_join(svc_worker_tid[i], NULL);
	}

	freeQueue(svcsock_queue);

	return 0;
}
