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
#include "bm_timediff.h"

static int stop = 0;
static int count = 0;

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
    char buffer[2048];
	char sockname[64];

	sprintf(sockname, "/tmp/client_%d.sock", id);

	int c = count;

	while (c--) {
		if (fd < 0) {
			fd = uds_connect_sock(sockname);
		}

		ret = write(fd, &buffer, sizeof(buffer));
		if (ret <= 0) {
			perror("write");
			close(fd);
			fd = -1;
			exit(1);
		}
		ret = read(fd, &buffer, sizeof(buffer));
		if (ret <= 0) {
			perror("read");
			close(fd);
			fd = -1;
			exit(1);
		}
	}
	printf("%d/%d\n", c, count);
	return NULL;
}

int main(int argc, char **argv)
{
	int thread_num = atoi(argv[1]);
	count = atoi(argv[2]);
	pthread_t *tid = malloc(sizeof(pthread_t) * thread_num);
	int i;
	struct bm_timediff t;

	init_bm_timediff(&t);
	for (i=0; i<thread_num; i++) {
		pthread_create(&tid[i], NULL, worker_thread, (void *)i);
	}
	for (i=0; i<thread_num; i++) {
		pthread_join(tid[i], NULL);
	}
	check_bm_timediff("", &t, 1);
    return 0;
}
