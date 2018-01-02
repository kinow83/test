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
#include <assert.h> 

#include "dprintf.h"
#include "pbio_data.h"

static void debug_pnindex(pbio_data_queue_t *q)
{
	pbio_index_t *pbindex, *p;
	int n = 0;
	cds_list_for_each_entry_safe(pbindex, p, &q->unused_head, node) {
		DPRINTF("unused: %d, index: %d\n", n++, pbindex->index);
	}
	cds_list_for_each_entry_safe(pbindex, p, &q->used_head, node) {
		DPRINTF("used: %d,  index: %d\n", n++, pbindex->index);
	}
}

pbio_data_queue_t *new_pbio_data_queue(int shmkey, uint16_t max_size, int is_allocator)
{
	pbio_data_queue_t *q = NULL;
	pbio_index_t *pbindex;
	int shmid, i;

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
	q->queue = (pbio_data_t *)shmat(shmid, (void *)0, 0);
	q->is_allocator = is_allocator;
#ifdef PBIO_DEBUG
	q->num_unused = 0;
	q->num_used = 0;
#endif
	CDS_INIT_LIST_HEAD(&q->unused_head);
	CDS_INIT_LIST_HEAD(&q->used_head);

	if (q->is_allocator) {
		for (i=0; i<max_size; i++) {
			pbindex = malloc(sizeof(*pbindex));
			pbindex->index = i;
			pbindex->timestamp = 0;
			cds_list_add_tail(&pbindex->node, &q->unused_head);
		}
//		debug_pnindex(q);
	}

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
	struct cds_list_head *pos, *p;

	// remove shared memory
	shmctl(q->shmid, IPC_RMID, 0);

	if (q->is_allocator) {
		// remove index list
		cds_list_for_each_safe(pos, p, &q->unused_head) {
			cds_list_del(pos);
			free(cds_list_entry(pos, pbio_index_t, node));
		}
		cds_list_for_each_safe(pos, p, &q->used_head) {
			cds_list_del(pos);
			free(cds_list_entry(pos, pbio_index_t, node));
		}
	}
	free(q);
	*pq = NULL;
}

size_t gc_unused_pbio_data_queue(pbio_data_queue_t *q, time_t timeout)
{
#if 0
	if (q->is_allocator == 0) {
		assert(q->is_allocator == 0);
	}
#endif
	if (cds_list_empty(&q->used_head)) {
		return 0;
	}
#ifdef PBIO_DEBUG
	DPRINTF("[before] unused: %d, used: %d\n", q->num_unused, q->num_used);
#endif
	pbio_index_t *pbindex, *p;
	cds_list_for_each_entry_safe(pbindex, p, &q->used_head, node) {
		if (pbindex->timestamp > timeout) {
			pbindex->timestamp = 0;
			cds_list_move(&pbindex->node, &q->unused_head);
#ifdef PBIO_DEBUG
			q->num_unused++;
			q->num_used--;
#endif
		}
	}
#ifdef PBIO_DEBUG
	DPRINTF("[after ] unused: %d, used: %d\n", q->num_unused, q->num_used);
#endif
}

pbio_data_t *alloc_pbio_data_queue(pbio_data_queue_t *q, uint16_t *index)
{
#if 0
	if (q->is_allocator == 0) {
		assert(q->is_allocator == 0);
	}
#endif
	pbio_index_t *pbindex;

	if (cds_list_empty(&q->unused_head)) {
		return NULL;
	}
	pbindex = cds_list_first_entry(&q->unused_head, pbio_index_t, node);
	pbindex->timestamp = time(NULL);
	cds_list_move(&pbindex->node, &q->used_head);
	*index = pbindex->index;
#ifdef PBIO_DEBUG
	q->num_unused--;
	q->num_used++;
#endif
	DPRINTF("allocated [%d]\n", *index);

//	debug_pnindex(q);
	return &q->queue[pbindex->index];
}

void dealloc_pbio_data_queue(pbio_data_queue_t *q, uint16_t index)
{
#if 0
	if (q->is_allocator == 0) {
		assert(q->is_allocator == 0);
	}
#endif
	pbio_index_t *pbindex, *p;
	cds_list_for_each_entry_safe(pbindex, p, &q->used_head, node) {
		if (pbindex->index == index) {
			pbindex->timestamp = 0;
			cds_list_move(&pbindex->node, &q->unused_head);
#ifdef PBIO_DEBUG
			q->num_unused++;
			q->num_used--;
#endif
			DPRINTF("deallocated [%d]\n", index);
			break;
		}
	}
}

pbio_data_t *peek_pbio_data_queue(pbio_data_queue_t *q, uint16_t index)
{
	return &q->queue[index];
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
	DPRINTF("%s (%d, %ld)\n", d->data, d->used, d->datalen);

	destroy_pbio_data(&q);
}
#endif
