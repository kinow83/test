#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>     // sleep()
#include <sys/ipc.h>
#include <sys/shm.h>

#include <pthread.h>
#include <semaphore.h>

typedef struct shLock
{
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexAttr;
	char buffer[100];
}shLock;


void main()
{
	int shmid;
	key_t key = 5678;
	shLock* lockptr;

	if ((shmid = shmget(key, sizeof(shLock), IPC_CREAT | 0666)) < 0) 
	{
		printf("[readop] error : cannot creat shm\n");
	}
	if ((lockptr = (shLock*)shmat(shmid, NULL, 0)) == (shLock *) -1) 
	{
		printf("[readop] error : failed attach memory\n");
	}

	while(1)
	{
		if( pthread_mutex_lock( &lockptr->mutex ) == 0 )
		{
			sleep(3);
			printf("[readop] shLock->buffer is [%s]",lockptr->buffer);
		}
		printf("[readop] run...\n");
		pthread_mutex_unlock( &lockptr->mutex );
		sleep(1);
	}
}
