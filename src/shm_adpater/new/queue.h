#ifndef __QUEUE__
#define __QUEUE__

typedef struct queue_t queue_t;

queue_t* new_queue(int capacity);
int enqueue(queue_t *q, void *value);
void* dequeue(queue_t *q);
void free_queue(queue_t *q);

#endif
