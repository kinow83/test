#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <string.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "ipcs.h"
#include "ipcs_shm.h"
#include "bm_timediff.h"

extern struct ipcs_ops ipc;

static void sig_handler(int signo)
{
	ipc.destroy();
	exit(1);
}

int main(int argc, char **argv)
{
	key_t SHMKEY;
	int shmid;
	void *shm_mem;
	shm_data_t *data;
	int loop = atoi(argv[1]);
	int idx = 0;
	int value;
	size_t page_size = getpagesize();
	size_t shm_size = sizeof(shm_data_t) * TEST_LOOP;
	int i, k;

	SHMKEY = ftok("/dev/null", 'K');
	if (SHMKEY == -1) {
		perror("ftok");
		exit(1);
	}

	signal(SIGINT, (void *)sig_handler);

	shm_size = aligned_size(shm_size, page_size);

	printf("shm_size = %ld\n", shm_size);
	shmid = shmget((key_t)SHMKEY, shm_size, IPC_CREAT|IPC_EXCL|0666);
	if (shmid < 0) {
		perror("shmget");
		shmid = shmget(SHMKEY, shm_size, 0666);
		if (shmid < 0) {
			shmctl(shmid, IPC_RMID, NULL);
			perror("shmget");
			exit(1);
		}
	}
	shm_mem = shmat(shmid, (void *)0, 0);
	if (shm_mem < 0) {
		perror("shmat");
		exit(1);
	}

	ipc.open();

	data = (shm_data_t*)shm_mem;

	struct bm_timediff bm;

	init_bm_timediff(&bm);

	for (i=0; i<loop; i++) {
		idx = 0;
		for (k=0; k<TEST_LOOP; k++) {
			ipc.wait();
			value = data[idx].value;
//			printf("%d\n", value);
			idx++;

		}
	}

	check_bm_timediff("end", &bm, 1);

	ipc.destroy();

	return 0;
} 
