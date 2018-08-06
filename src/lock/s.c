#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
	int i=1;

	char d[] = "Start Test....";

	if ((shmid = shmget(key, sizeof(shLock), IPC_CREAT | 0666)) < 0) 
	{
		printf("[writeop] error : cannot creat shm\n");
	}

	if ((lockptr = (shLock*)shmat(shmid, NULL, 0)) == (shLock *) -1) 
	{
		printf("[writeop] error : failed attach memory\n");
	}

	memset( lockptr->buffer,0,sizeof( lockptr->buffer) );

	memcpy( lockptr->buffer,d,strlen( d ) );

	if( pthread_mutexattr_init( &lockptr->mutexAttr ) == 0 )
	{
		printf("[writeop] success : shared mutex attr intialized\n");
	}

	pthread_mutexattr_setrobust(&lockptr->mutexAttr, PTHREAD_MUTEX_ROBUST);

	if( pthread_mutexattr_setpshared( &lockptr->mutexAttr,PTHREAD_PROCESS_SHARED) == 0 )
	{
		printf("[writeop] success : shared mutexattribute set\n");
	}

	if( pthread_mutex_init( &lockptr->mutex, &lockptr->mutexAttr) == 0 )
	{
		printf("[writeop] success : shared mutex intialized\n");
	}

	while(1)
	{
		char buff[128]={0,};
		int waitsec=3;
		int ret, e;

		sleep(1);

		sprintf(buff,"write test string is [%d]\n", i);
		ret = pthread_mutex_lock( &lockptr->mutex );
		if (ret == 0 ) {
			memcpy( lockptr->buffer, buff ,strlen( buff ) );
			printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
		} else if (ret == EOWNERDEAD) {
			printf("EOWNEREAD\n");
		
			e = pthread_mutex_consistent( &lockptr->mutex );
			if (e) {
				errno = e;
				perror("pthread_mutex_consistent"); exit(1);
			}
			e = pthread_mutex_unlock( &lockptr->mutex );
			if (e) {
				errno = e;
				perror("pthread_mutex_unlock"); exit(1);
			}
		}
		pthread_mutex_unlock( &lockptr->mutex ) ;
		printf("[writeop] run...\n");
		i++;
	}
}
