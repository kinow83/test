#ifndef __PBIO_DATA__
#define __PBIO_DATA__

#include "urcu/list.h"

#define PBIO_DATA_SIZE 1500

typedef struct pbio_data_t {
	uint8_t used;
	uint8_t data[PBIO_DATA_SIZE];
	size_t datalen;
} pbio_data_t;

typedef struct pbio_index_t {
	uint16_t index;
	time_t timestamp;

	struct cds_list_head node;
} pbio_index_t;

typedef struct pbio_data_queue_t {
	int shmid;
	int shmkey;
	uint16_t max_size;
	int is_allocator;
	pbio_data_t *queue;

	struct cds_list_head unused_head;
	size_t num_unused;
	struct cds_list_head used_head;
	size_t num_used;
} pbio_data_queue_t;


pbio_data_queue_t *new_pbio_data_queue(int shmkey, uint16_t max_size, int is_allocator);
void destroy_pbio_data(pbio_data_queue_t **pq);
pbio_data_t *alloc_pbio_data_queue(pbio_data_queue_t *q, uint16_t *index);
void dealloc_pbio_data_queue(pbio_data_queue_t *q, uint16_t index);
pbio_data_t *peek_pbio_data_queue(pbio_data_queue_t *q, uint16_t index);
size_t gc_unused_pbio_data_queue(pbio_data_queue_t *q, time_t timeout);

#endif
