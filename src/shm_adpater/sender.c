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

pthread_mutex_t csocks_mutex = PTHREAD_MUTEX_INITIALIZER;
struct Queue* csocks_queue;

void* worker_thread(void *arg)
{
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

			printf("get csock: %d\n", *csock);
			noop = 0;
		}
		pthread_mutex_unlock(&csocks_mutex);
		break;
	}

	while (1) {
		// io adapter
		char buf[100];
		size_t n;
		n = read(*csock, buf, sizeof(buf));
		if (n <= 0) {
			close(*csock);
			free(csock);
			break;
		}
		printf("%s\n", buf);
	}
	goto retry;

	return NULL;
}

void* accept_thread(void *arg)
{
	struct sockaddr_in c_addr;
	int s_sock, c_sock;
	int *psock;
	int n;
	socklen_t c_addr_len = sizeof(c_addr);
	
	s_sock = tcp_listen_sock(8800);
	printf("new accept socket: %d\n", s_sock);

	while (1)
	{
		c_sock = accept(s_sock, (struct sockaddr*)&c_addr, &c_addr_len);
		printf("accept new client socket: %d\n", c_sock);

		psock = malloc(sizeof(int));
		*psock = c_sock;

		pthread_mutex_lock(&csocks_mutex);
		{
			n = enqueue(csocks_queue, psock);
			printf("enqueue sock:%d in %d\n", *psock, n);
		}
		pthread_mutex_unlock(&csocks_mutex);
	}
	return NULL;
}

int main(int argc, char **argv)
{
	const static int num_workers = 2;
	struct sockaddr_in c_addr;
	int s_sock, c_sock;
	pthread_t accept_tid;
	pthread_t worker_tid[num_workers];
	int i;

	csocks_queue = newQueue(10);

	pthread_create(&accept_tid, NULL, accept_thread, NULL);

	for (i=0; i<num_workers; i++) {
		pthread_create(&worker_tid[i], NULL, worker_thread, NULL);
	}

	pthread_join(accept_tid, NULL);
	for (i=0; i<num_workers; i++) {
		pthread_join(worker_tid[i], NULL);
	}

	freeQueue(csocks_queue);
	return 0;
}
