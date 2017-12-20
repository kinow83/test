#ifndef __SIMPLE_QUEUE__
#define __SIMPLE_QUEUE__

// https://gist.github.com/rdleon/d569a219c6144c4dfc04366fd6298554

struct Queue;

extern struct Queue* newQueue(int capacity);

extern int enqueue(struct Queue *q, void *value);

extern void* dequeue(struct Queue *q);

extern void freeQueue(struct Queue *q);

#endif

