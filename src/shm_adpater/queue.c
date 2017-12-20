#include <stdlib.h>
#include "queue.h"

struct Node {
	void *value;
	struct Node *next;
};

struct Queue {
	int size;
	int max_size;
	struct Node *head;
	struct Node *tail;
};


struct Queue* newQueue(int capacity)
{
	struct Queue *q;
	q = malloc(sizeof(struct Queue));

	if (q == NULL) {
		return q;
	}

	q->size = 0;
	q->max_size = capacity;
	q->head = NULL;
	q->tail = NULL;

	return q;
}

int enqueue(struct Queue *q, void *value)
{
	if ((q->size + 1) > q->max_size) {
		return q->size;
	}

	struct Node *node = malloc(sizeof(struct Node));

	if (node == NULL) {
		return q->size;
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

void* dequeue(struct Queue *q)
{
	if (q->size == 0) {
		return NULL;
	}

	void *value = NULL;
	struct Node *tmp = NULL;

	value = q->head->value;
	tmp = q->head;
	q->head = q->head->next;
	q->size -= 1;

	free(tmp);

	return value;
}

void freeQueue(struct Queue *q)
{
	if (q == NULL) {
		return;
	}

	while (q->head != NULL) {
		struct Node *tmp = q->head;
		q->head = q->head->next;
		if (tmp->value != NULL) {
			free(tmp->value);
		}

		free(tmp);
	}

	if (q->tail != NULL) {
		free(q->tail);
	}

	free (q);
}
