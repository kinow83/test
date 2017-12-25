#ifndef __PBIO_DATA__
#define __PBIO_DATA__

#define PBIO_DATA_SIZE 1500

typedef struct pbio_data_t {
	uint8_t used;
	uint8_t data[PBIO_DATA_SIZE];
	size_t datalen;
} pbio_data_t;

typedef struct pbio_data_queue_t {
	int shmid;
	int shmkey;
	uint16_t max_size;
	uint16_t pos;
	pbio_data_t *queue;
} pbio_data_queue_t;


pbio_data_queue_t *new_pbio_data_queue(int shmkey, uint16_t max_size);
void destroy_pbio_data(pbio_data_queue_t **pq);
pbio_data_t *alloc_pbio_data_queue(pbio_data_queue_t *q, uint16_t *index);
int put_pbio_data_queue(pbio_data_queue_t *q, void *data, size_t datalen, uint16_t *index);
pbio_data_t *peek_pbio_data_queue(pbio_data_queue_t *q, uint16_t index);
void used_pbio_data_queue(pbio_data_queue_t *q, uint16_t index);
void unused_pbio_data_queue(pbio_data_queue_t *q, uint16_t index);

#endif
