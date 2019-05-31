#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <glib-2.0/glib.h>
#include "dut.h"

static GAsyncQueue **g_async_queues = NULL;
static int g_worker_cnt;

void init_queue(int queue_len, int worker_cnt)
{
	int i = 0;
	queue_len = queue_len;
	g_async_queues = calloc(sizeof(GAsyncQueue *), worker_cnt);
	for (i = 0; i < worker_cnt; i++) {
		g_async_queues[i] = g_async_queue_new ();
	}
	g_worker_cnt = worker_cnt;
}

int enqueue_queue(void *data, int queue_id)
{
	g_async_queue_push(g_async_queues[queue_id], data);
	return 0;
}

int dequeue_queue(void **data, int queue_id)
{
	*data = g_async_queue_pop(g_async_queues[queue_id]);
	return 1;
}

struct dut_ops queue_dut = {
		.init = init_queue,
		.enqueue = enqueue_queue,
		.dequeue = dequeue_queue,
};
