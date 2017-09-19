/*
 * work_queue.h
 *
 *  Created on: 2017. 9. 17.
 *      Author: root
 */

#ifndef LIBEVENT_WIKI_WORK_QUEUE_H_
#define LIBEVENT_WIKI_WORK_QUEUE_H_

#include <pthread.h>

struct worker_queue;

struct worker {
	pthread_t pid;
	char terminated;
	struct worker_queue *worker_queue;
	struct worker *prev;
	struct worker *next;
};

struct job {
	void (*run)(struct job *);
	void *ctx;
	struct job *prev;
	struct job *next;
};

struct worker_queue {
	int num_jobs;
	struct worker *worker_list;
	struct job *job_list;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

int worker_queue_init(struct worker_queue *worker_queue, int num_workers);

void worker_queue_shutdown(struct worker_queue *worker_queue);

void worker_queue_add_job(struct worker_queue *worker_queue, struct job  *job);

void worker_queue_lock(struct worker_queue *worker_queue);
void worker_queue_unlock(struct worker_queue *worker_queue);
void worker_queue_wakeup_all(struct worker_queue *worker_queue);
void worker_queue_wakeup_one(struct worker_queue *worker_queue);
void worker_queue_wait(struct worker_queue *worker_queue);

#endif /* LIBEVENT_WIKI_WORK_QUEUE_H_ */
