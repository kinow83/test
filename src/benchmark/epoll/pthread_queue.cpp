/*  Producer Consumer example using pthreads

	This program implements the classical producer consumer problem
	using POSIX standard pthreads.  It compiles and runs on most
	Unix systems.
	written by Ken Williams at NC A&T SU on 1/26/03 */
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include "pthread_queue.h"

#define QNUM 8
int	q[QNUM];					// queue to hold produced numbers
int	first  = 0;					// index into q of next free space
int last   = 0;					// index into q of next available number
int	numInQ = 0;					// number of items in the queue
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	// mutual exclusion lock
pthread_mutex_t empty = PTHREAD_MUTEX_INITIALIZER;	// synchronization lock

/* putOnQ is called by the producer threads to put a number on the queue */
void putOnQ(int x) 
{
	pthread_mutex_lock(&mutex);		// lock access to the queue
	q[first] = x;					// put item on the queue
	first = (first+1) % QNUM;
	numInQ++;						// increment queue size
	pthread_mutex_unlock(&mutex);	// unlock queue
	pthread_mutex_unlock(&empty);	// start a waiting consumer
}

/* getOffQ is called by consumer threads to retrieve a number
   from the queue.  The thread will be suspended if there is
   nothing on the queue. */
int getOffQ(void) 
{
	int thing;						// item removed from the queue
	/* wait if the queue is empty. */
	while (numInQ == 0) pthread_mutex_lock(&empty);
	pthread_mutex_lock(&mutex);		// lock access to the queue
	thing = q[last];				// get item from the queue
	last = (last+1) % QNUM;
	numInQ--;						// decrement queue size
	pthread_mutex_unlock(&mutex);	// unlock queue
	return thing;
}
