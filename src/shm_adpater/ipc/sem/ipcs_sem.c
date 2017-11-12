#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include "ipcs.h"


sem_t *mysem;

static void create_sem(void)
{
	int initilize = 0;
	if (access("sem.lock", F_OK) == 0) {
		initilize = 1;
	} else {
		initilize = 0;
	}

	if (initilize == 0) {
		sem_unlink("mysem");
		mysem = sem_open("mysem", O_CREAT, 0777, 0);
		if (!mysem) {
			perror("sem_open");
			exit(1);
		}
		initilize = 1;
		system("touch sem.lock");
	} else {
		mysem = sem_open("mysem", 0, 0777, 0);
		if (mysem == SEM_FAILED) {
			perror("sem_open");
			exit(1);
		}
	}
}

static void open_sem(void)
{
	create_sem();
}

static void post_sem(void)
{
	if (sem_post(mysem)) {
		perror("sem_post");
		exit(1);
	}
}

static void wait_sem(void)
{
	if (sem_wait(mysem)) {
		perror("sem_wait");
		exit(1);
	}
}

static int wait_timeout_sem(uint32_t msec)
{
	struct timespec ts = {
		.tv_sec = 0, 
		.tv_nsec = msec * 1000 
	};
	if (sem_timedwait(mysem,  &ts)) {
		if (errno == ETIMEDOUT) {
			return 0;
		}
		return 1;
	}
	return 0;
}

static void destroy_sem()
{
	if (mysem) {
		sem_destroy(mysem);
		mysem = NULL;
	}
	unlink("sem.lock");
}

struct ipcs_ops ipc = {
	.create = create_sem,
	.open = open_sem,
	.post = post_sem,
	.wait = wait_sem,
	.wait_timeout = wait_timeout_sem,
	.destroy = destroy_sem,
};

