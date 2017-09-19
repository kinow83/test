/*
 * work_queue.c
 *
 *  Created on: 2017. 9. 17.
 *      Author: root
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "work_queue.h"

#define LLIST_ADD(item, list) { \
		item->prev = NULL; \
		item->next = list; \
		list = item; \
}

#define LLIST_RM(item, list) { \
	if (item->prev) item->prev->next = item->next; \
	if (item->next) item->next->prev = item->prev; \
	if (list == item) list = item->next; \
	item->prev = item->next = NULL; \
}

inline void worker_queue_lock(struct worker_queue *worker_queue) {
	pthread_mutex_lock(&worker_queue->mutex);
}

inline void worker_queue_unlock(struct worker_queue *worker_queue) {
	pthread_mutex_unlock(&worker_queue->mutex);
}

inline void worker_queue_wakeup_all(struct worker_queue *worker_queue) {
	pthread_cond_broadcast(&worker_queue->cond);
}

inline void worker_queue_wakeup_one(struct worker_queue *worker_queue) {
	pthread_cond_signal(&worker_queue->cond);
}

inline void worker_queue_wait(struct worker_queue *worker_queue) {
	pthread_cond_wait(&worker_queue->cond, &worker_queue->mutex);
}

static void *worker_run(void *ctx)
{
	struct worker *worker = (struct worker*)ctx;
	struct worker_queue *worker_queue = worker->worker_queue;
	struct job *job;

	while (1)
	{
		worker_queue_lock(worker_queue);
		while (!worker_queue->job_list) {
			// empty jobs
			if (worker->terminated) break;

			// worker_queue_wait call unlock, and wait for signal
			worker_queue_wait(worker_queue);
			// after return worker_queue_wait, again lock
		}

		if (worker->terminated) break;

		job = worker_queue->job_list;
		if (job) {
			LLIST_RM(job, worker_queue->job_list);
		}
		worker_queue_unlock(worker_queue);

		// can not get a job, then nothing to do
		if (!job) continue;

		job->run(job);
	}

	free(worker);
	pthread_exit(job);
}

int worker_queue_init(struct worker_queue *worker_queue, int num_workers)
{
	int i;
	struct worker *worker;

	if (num_workers < 1) num_workers = 1;

	memset(worker_queue, 0, sizeof(struct worker_queue));
	pthread_mutex_init(&worker_queue->mutex, NULL);
	pthread_cond_init(&worker_queue->cond, NULL);

	for (i=0; i<num_workers; i++) {
		worker = (struct worker *)malloc(sizeof(struct worker));
		if (!worker) {
			perror("worker_queue_init()");
			return -1;
		}

		memset(worker, 0, sizeof(struct worker));
		worker->worker_queue = worker_queue;
		if (pthread_create(&worker->pid, NULL, worker_run, (void *)worker)) {
			perror("pthread_create()");
			free(worker);
			return -1;
		}

		LLIST_ADD(worker, worker_queue->worker_list);
	}
	return 0;
}

void worker_queue_shutdown(struct worker_queue *worker_queue)
{
	struct worker *worker;

	for (worker = worker_queue->worker_list;
			worker != NULL;
			worker = worker->next) {
		worker->terminated = 1;
	}

	worker_queue_lock(worker_queue);
	{
		worker_queue->worker_list = NULL;
		worker_queue->job_list = NULL;
		worker_queue_wakeup_all(worker_queue);
	}
	worker_queue_unlock(worker_queue);
}

void worker_queue_add_job(struct worker_queue *worker_queue, struct job  *job)
{
	worker_queue_lock(worker_queue);
	{
		// add the job to the job list.
		LLIST_ADD(job, worker_queue->job_list);
		// notify a worker
		worker_queue_wakeup_one(worker_queue);
	}
	worker_queue_unlock(worker_queue);
}

