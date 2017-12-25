#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h"


typedef struct q_node_t {
	void *value;
	struct q_node_t *next;
} q_node_t;

typedef struct queue_t {
	int size;
	int max_size;
	q_node_t *head;
	q_node_t *tail;
} queue_t;


queue_t* new_queue(int capacity)
{
	queue_t *q;
	q = malloc(sizeof(queue_t));

	if (q == NULL) {
		return NULL;
	}

	q->size = 0;
	q->max_size = capacity;
	q->head = NULL;
	q->tail = NULL;

	return q;
}

int enqueue(queue_t *q, void *value)
{
	if ((q->size + 1) > q->max_size) {
		return -1;
	}

	q_node_t *node = malloc(sizeof(q_node_t));

	if (node == NULL) {
		return -1;
	}

	node->value = value;
	node->next = NULL;

	if (q->head == NULL) {
		q->head = node;
		q->tail = node;
		q->size = 1;

		return q->size;
	}


	q->tail->next = node;
	q->tail = node;
	q->size += 1;

	return q->size;
}

void* dequeue(queue_t *q)
{
	if (q->size == 0) {
		return NULL;
	}

	void *value = NULL;
	q_node_t *tmp = NULL;

	value = q->head->value;
	tmp = q->head;
	q->head = q->head->next;
	q->size -= 1;

	free(tmp);

	return value;
}

void free_queue(queue_t *q)
{
	if (q == NULL) {
		return;
	}

	while (q->head != NULL) {
		q_node_t *tmp = q->head;
		q->head = q->head->next;
		if (tmp->value != NULL) {
			free(tmp->value);
		}

		free(tmp);
	}

	if (q->tail != NULL) {
		free(q->tail);
	}

	free(q);
}

#if 0
int main(int argc, char **argv)
{
	int a, b, c, d, e;
	int n;

	queue_t *q = new_queue(5);

	a = 1;
	n = enqueue(q, &a);
	assert(n >= 0);

	b = 2;
	n = enqueue(q, &b);
	assert(n >= 0);

	c = 3;
	n = enqueue(q, &c);
	assert(n >= 0);

	d = 4;
	n = enqueue(q, &d);
	assert(n >= 0);

	e = 5;
	n = enqueue(q, &e);
	assert(n >= 0);

	int *p;
	p = dequeue(q);
	assert(p != NULL);
	printf("%d\n", *p);

	p = dequeue(q);
	assert(p != NULL);
	printf("%d\n", *p);

	p = dequeue(q);
	assert(p != NULL);
	printf("%d\n", *p);

	p = dequeue(q);
	assert(p != NULL);
	printf("%d\n", *p);

	p = dequeue(q);
	assert(p != NULL);
	printf("%d\n", *p);

	p = dequeue(q);
	assert(p != NULL);
	printf("%d\n", *p);

	return 0;
}
#endif
