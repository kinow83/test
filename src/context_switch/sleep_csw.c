#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/poll.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>

int who1 = RUSAGE_SELF;
int who2 = RUSAGE_CHILDREN;

void show_csw(const char *msg, int who, struct rusage *usage1, struct rusage *usage2)
{
	getrusage(who, usage2);
	printf("%s\tvoluntary context switches:   %ld\n", msg, usage2->ru_nvcsw  - usage1->ru_nvcsw);
	printf("%s\tinvoluntary context switches: %ld\n", msg, usage2->ru_nivcsw - usage1->ru_nivcsw);
}

int timeout = 1;
int core = 4;

void *polling(void *arg)
{
	int n = 0;
	int i, j;
	struct rusage usage1;
	struct rusage usage2;

	while (1) {
#if 1
		for (i=0; i<timeout; i++) {
			getrusage(who1, &usage1);
			for (j=0; j<=100000000; j++) {
				n += i+j;
			}
			show_csw("poll", who1, &usage1, &usage2);
		}
#else
	usleep(timeout);
#endif
		break;
	}
	return NULL;
}

int main(int argc, char **argv) {
	struct rusage usage;
	int i, j, k = 0;
	timeout = atoi(argv[1]);
	core = atoi(argv[2]);
	struct rusage usage1;
	struct rusage usage2;

	getrusage(who1, &usage1);

	pthread_t tid[core];

	for (i=0; i<core; i++) {
		pthread_create(&tid[i], NULL, polling, NULL);
	}

	for (i=0; i<core; i++) {
		pthread_join(tid[i], NULL);
	}

	show_csw("main", who1, &usage1, &usage2);
	return 0;
}
