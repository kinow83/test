#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <pthread.h>
#include "sock.h"

static int stop = 0;

static void die(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static void *worker_thread(void *arg)
{
	int id = (int)arg;
	int ret;
	int fd = -1;
	uint32_t data;
	uint64_t cnt = 0;

	while (stop == 0) {
		if (fd < 0) {
			fd = uds_connect_sock("/tmp/echo_0.sock");
		}

		ret = read(fd, &data, sizeof(data));
		if (ret <= 0) {
			perror("read");
			close(fd);
			fd = -1;
			exit(1);
		}
		ret = write(fd, &data, sizeof(data));
		if (ret <= 0) {
			perror("write");
			close(fd);
			fd = -1;
			exit(1);
		}
#ifdef DEBUG
		printf("read / write: %d\n", data);
#endif

		cnt++;
	}
	return NULL;
}

int main(int argc, char **argv)
{
	int thread_num = atoi(argv[1]);
	pthread_t *tid = malloc(sizeof(pthread_t) * thread_num);
	int i;

	for (i=0; i<thread_num; i++) {
		pthread_create(&tid[i], NULL, worker_thread, (void *)i);
	}
	for (i=0; i<thread_num; i++) {
		pthread_join(tid[i], NULL);
	}
    return 0;
}
