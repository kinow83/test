#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

typedef struct shm_struct {
	pthread_mutex_t mtx;
	pthread_cond_t cond;
	int cnt;
} shm_struct_t;

shm_struct_t *g_shm;
pthread_mutexattr_t mtx_attr;
pthread_condattr_t cond_attr;

int main(int argc, char **argv)
{
	sem_t *bin_sem1;
	sem_t *bin_sem2;
	int ret;
	int id = atoi(argv[1]);
	int shm_size;
	int shmid;
	int cnt = 0;

	shmid = shmget((key_t)1234, sizeof(shm_struct_t), 0666|IPC_CREAT);
	if (shmid == -1) {
		perror("shmget");
		exit(1);
	}
	g_shm = shmat(shmid, NULL, 0);
	if (g_shm == (void *)-1) {
		perror("shmat");
		exit(1);
	}

	pthread_mutex_t mtx;
	pthread_mutexattr_init(&mtx_attr);
	ret = pthread_mutexattr_setpshared(&mtx_attr, PTHREAD_PROCESS_SHARED);
	if (ret) {
		perror("pthread_mutexattr_setpshared");
		exit(1);
	}

	g_shm->cnt = 0;

	pthread_mutex_init(&mtx, &mtx_attr);

	if (id == 1) bin_sem1 = sem_open("mysem1", O_CREAT, 0777, 0);
	else         bin_sem1 = sem_open("mysem1", 0, 0777, 0);

	if (id == 1) bin_sem2 = sem_open("mysem2", O_CREAT, 0777, 0);
	else         bin_sem2 = sem_open("mysem2", 0, 0777, 0);

	int call = 0;

	while (1) {
//		pthread_mutex_lock(&mtx);
		if (id == 1) {
			sem_wait(bin_sem1);
			printf("%d\n", ++call);
			usleep(10000);
//			sem_post(bin_sem2);
		} else {
			sem_post(bin_sem1);
			printf("%d\n", ++call);
			usleep(10);
//			sem_wait(bin_sem2);
		}
//		pthread_mutex_unlock(&mtx);
#if 0
		pthread_mutex_lock(&mtx);
		{
			g_shm->cnt = id;
			printf("%d\n", g_shm->cnt);
		}
		pthread_mutex_unlock(&mtx);
#endif
	}

	sem_destroy(bin_sem1);
	sem_destroy(bin_sem2);

	return 0;
}
