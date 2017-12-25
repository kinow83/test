#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/shm.h> 
#include <errno.h> 

#include "pbio_data.h"

pbio_data_queue_t *new_pbio_data_queue(int shmkey, uint16_t max_size)
{
	pbio_data_queue_t *q = NULL;
	int shmid;

	q = calloc(1, sizeof(*q));

	shmid = shmget((key_t)shmkey, sizeof(pbio_data_t)*max_size, 0666|IPC_CREAT);
	if (shmid == -1) {
		shmid = shmget((key_t)shmkey, sizeof(pbio_data_t)*max_size, 0666);
		if (shmid == -1) {
			goto error;
		}
	}
	q->shmid = shmid;
	q->shmkey = shmkey;
	q->max_size = max_size;
	q->pos = -1;
	q->queue = (pbio_data_t *)shmat(shmid, (void *)0, 0);
	return q;
error:
	if (shmid != -1) {
		shmdt(q->queue);
	}
	if (q) free(q);
	return NULL;
}

void destroy_pbio_data(pbio_data_queue_t **pq)
{
	pbio_data_queue_t *q = *pq;
	shmctl(q->shmid, IPC_RMID, 0);
	free(q);
	*pq = NULL;
}

pbio_data_t *alloc_pbio_data_queue(pbio_data_queue_t *q, uint16_t *index)
{
	uint16_t i;
	uint16_t next;

	if (q->pos+1 >= q->max_size) {
		next = 0;
		goto search;
	}
	if (q->queue[q->pos+1].used == 1) {
		next = q->pos+2;
		goto search;
	}
	next = q->pos+1;
	goto done;
search:
	for (i=next; i<q->max_size; i++) {
		if (q->queue[next].used == 0) {
			next = i;
			goto done;
		}
	}
	return NULL;
done:
	*index = q->pos = next;
	return &q->queue[q->pos];
}

int put_pbio_data_queue(pbio_data_queue_t *q, void *data, size_t datalen, uint16_t *index)
{
	pbio_data_t *pdata;
	int i;
	int next;

	if (q->pos+1 >= q->max_size) {
		next = 0;
		goto search;
	}
	if (q->queue[q->pos+1].used == 1) {
		next = q->pos+2;
		goto search;
	}
	next = q->pos+1;
	goto done;
search:
	for (i=next; i<q->max_size; i++) {
		if (q->queue[next].used == 0) {
			next = i;
			goto done;
		}
	}
	return -1;
done:
	pdata = &q->queue[next];
	memcpy(pdata->data, data, datalen);
	pdata->used = 0;
	pdata->datalen = datalen;
	*index = q->pos = next;
	return 0;
}

pbio_data_t *peek_pbio_data_queue(pbio_data_queue_t *q, uint16_t index)
{
	if (index >= q->max_size) {
		return NULL;
	}
	return &q->queue[index];
}

void used_pbio_data_queue(pbio_data_queue_t *q, uint16_t index)
{
	if (index >= q->max_size) {
		return;
	}
	q->queue[index].used = 1;
}

void unused_pbio_data_queue(pbio_data_queue_t *q, uint16_t index)
{
	if (index >= q->max_size) {
		return;
	}
	q->queue[index].used = 0;
}
/* end [ queue ] ***********************************************/


#if 0
int main()
{
	pbio_data_t *d;
	uint16_t index;
	pbio_data_queue_t *q = new_pbio_data_queue(2345, 10);

	d = alloc_pbio_data_queue(q, &index);
	strcpy(d->data, "k1");
	d->used = 0;
	d->datalen = strlen("k1");



	d = peek_pbio_data_queue(q, &index);
	printf("%s (%d, %ld)\n", d->data, d->used, d->datalen);

	destroy_pbio_data(&q);
}
#endif
