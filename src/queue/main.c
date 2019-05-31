#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "dut.h"
#include "rte_atomic.h"

extern struct dut_ops ring_dut;
extern struct dut_ops queue_dut;
static struct dut_ops *dut;
static pthread_t *tid;
struct dut_stat {
	rte_atomic32_t ok;
	rte_atomic32_t bad;
};
static struct dut_stat dutstat;
static clock_t start, end;
int32_t n_loop = 100000;

static void *worker_thread(void *arg)
{
	int n_get;
	int id = (int *)arg;
	void *data;
	while (1) {
		n_get = dut->dequeue(&data, id);
		if (n_get > 0) {
			rte_atomic32_add(&dutstat.ok, n_get);
			if (dutstat.ok.cnt == n_loop) {
				end = clock();
				printf("%f\n", (double)(end - start)/CLOCKS_PER_SEC);
			}
//			printf("%d: OK get %d\n", id, n_get);
		} else {
			rte_atomic32_inc(&dutstat.bad);
			usleep(10);
//			printf("%d: BAD get %d\n", id, n_get);
		}
	}
	return NULL;
}

int main(int argc, char **argv)
{
	int n_thread = 4;
	int i, k;
	int opt;

	while (1) {
		opt = getopt(argc, argv, "m:n:t:");
		if (opt == EOF) break;
		switch (opt) {
		case 't':

			n_thread = atoi(optarg);
			break;
		case 'm':
			if (strcasecmp("ring", optarg) == 0) {
				dut = &ring_dut;
			} else if (strcasecmp("queue",optarg) == 0) {
				dut = &queue_dut;
			}
			break;
		case 'n':
			n_loop = atoi(optarg);
			break;
		default:
			exit(1);
		}
	}
	if (!dut){
		printf("unknown type\n");
		exit(1);
	}

	dut->init(256, n_thread);

	tid = calloc(sizeof(pthread_t) * n_thread, 1);

	for (i=0; i<n_thread; i++) {
		pthread_create(&tid[i], NULL, worker_thread, i);
	}

	char dd[1000];
	int next_id = 0;
	int queue_id;
	i = 0;

	start = clock();

	while (i < n_loop) {
		void *data = dd;
		for (k=0; k<n_thread; k++) {

			queue_id = (next_id + k) % n_thread;
			if (dut->enqueue(data, queue_id) == 0) {
//				printf("queue_id:%d OK\n", queue_id);
				next_id = queue_id + 1;
				break;
			}
		}
		if (k == n_thread) {
			printf("enqueue full!\n");
			usleep(10);
		} else {
			i++;
		}
	}


	while (1) {
		sleep(1);
		printf("OK:%u, BAD:%u\n", dutstat.ok.cnt, dutstat.bad.cnt);
	}
}
