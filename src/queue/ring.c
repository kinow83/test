#include <stdio.h>
#include "rte_ring.h"
#include <stdlib.h>
#include "dut.h"

/*
 * install lib dpdk developement
 * root@kaka-pc:~# apt install libdpdk-dev -y
 */

static struct rte_ring ** g_rings;
static int g_worker_cnt;

void init_ring(int queue_len, int worker_cnt) {
	int i = 0;
	g_rings = malloc(sizeof(struct rte_ring *) * worker_cnt);
	for (i = 0; i < worker_cnt; i++) {
		g_rings[i] = rte_ring_create("test_ring", queue_len, 0, RING_F_SC_DEQ /*| RING_F_SP_ENQ*/);
		if (!g_rings[i]) {
			printf("err> queue_len:%d\n", queue_len);
		}
	}
	g_worker_cnt = worker_cnt;
}

int enqueue_ring(void *data, int queue_id)
{
	return rte_ring_enqueue(g_rings[queue_id], data);
}

int dequeue_ring(void **data, int queue_id)
{
	return rte_ring_dequeue_burst(g_rings[queue_id], data, 16 /*many*/);
}

struct dut_ops ring_dut = {
		.init = init_ring,
		.enqueue = enqueue_ring,
		.dequeue = dequeue_ring,
};
