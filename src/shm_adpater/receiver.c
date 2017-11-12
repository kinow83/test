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

pthread_mutex_t xid_mutex = PTHREAD_MUTEX_INITIALIZER;
struct Queue* xid_queue;

extern io_linker cio_linker;

void* worker_thread(void *arg)
{
	while (1) {
		sleep(1);
	}
	return NULL;
}

void* accept_thread(void *arg)
{
	io_link iolink;
	io_index ioindex;

	// init io linker
	if (cio_linker.init(&iolink) < 0) {
		printf("error: failed to cio_linker init\n");
		return NULL;
	}

	while (1) {
		if (cio_linker.read_index(&iolink, &ioindex) < 0) {
			break;
		}
		printf("read idx: %d, xid:%04X\n", ioindex.index);
	}

	cio_linker.destroy(&iolink);
}

int main(int argc, char **argv)
{
	const static int num_workers = 2;
	pthread_t accept_tid;
	pthread_t worker_tid[num_workers];
	int i;

	xid_queue = newQueue(10);

	pthread_create(&accept_tid, NULL, accept_thread, NULL);

	for (i=0; i<num_workers; i++) {
		pthread_create(&worker_tid[i], NULL, worker_thread, NULL);
	}

	pthread_join(accept_tid, NULL);
	for (i=0; i<num_workers; i++) {
		pthread_join(worker_tid[i], NULL);
	}

	freeQueue(xid_queue);

	return 0;
}
