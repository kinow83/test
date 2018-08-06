#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>

typedef struct slock {
	pthread_mutex_t lock;
	pthread_mutexattr_t attr;
	int sum;
} slock;

int main(int argc, char **argv)
{
	key_t key = 0x1234;
	int id = atoi(argv[1]);
	int shmid;
	slock *pslock;

	shmid = shmget(key, sizeof(slock), IPC_CREAT|0666);
	if (shmid < 0) {
		perror("shmget");
		exit(1);
	}

	pslock = (slock*)shmat(shmid, NULL, 0);
	if (pslock == (slock*)-1) {
		perror("shmat");
		exit(1);
	}

	if (id == 1) {
		pthread_mutexattr_init(&pslock->attr);
		if (pthread_mutexattr_setpshared(&pslock->attr, PTHREAD_PROCESS_SHARED) < 0) {
			perror("ssssss");
			exit(1);
		}
		pthread_mutex_init(&pslock->lock, &pslock->attr);
	}
	
	int i;

	for (i=0; i<100; i++) {
		pthread_mutex_lock(&pslock->lock);

		printf("%d] %d\n", id, pslock->sum++);
		sleep(id);

		pthread_mutex_unlock(&pslock->lock);
	}
}

