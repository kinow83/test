#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include "atomic.h"

#ifdef TEST
	#ifndef atomic_dec
		#define atomic_dec(x) (x)--
	#endif
	#ifndef atomic_inc
		#define atomic_inc(x) (x)++
	#endif
#else
	#define atomic_inc(x) _uatomic_inc(&(x))
	#define atomic_dec(x) _uatomic_dec(&(x))
#endif

#define cloader_busy_wait() \
	do { \
		usleep(1); \
	} while (0)

#define cloader_not_busy_wait() \
	do { \
		usleep(100); \
	} while (0)

struct cloader_t;

typedef struct cloader_ops_t {
	void* (*load)(struct cloader_t *, void *);
	void (*release)(void *);
} cloader_ops_t;

typedef struct cloader_cfg_t {
	void *data;
} cloader_cfg_t;

typedef struct cloader_t {
	uint8_t is_load_called; // load function is called only once.
	cloader_ops_t ops;  // cloader operations
	cloader_cfg_t cfg1; // config #1
	cloader_cfg_t cfg2; // config #2
	cloader_cfg_t *cur; // current config
	int reload;         // now reloading ?
	uint32_t ref_count; // referece count of config data
	pthread_t reloader_tid; // pthread id of the first called reload. Not support reload multi-thread.
} cloader_t;


int cloader_init(cloader_t *c, cloader_ops_t *ops)
{
	if (ops->load == NULL) {
		return -1;
	}

	memset(c, 0, sizeof(*c));
	c->ops.load = ops->load;
	c->ops.release = ops->release;

	return 0;
}

void cloader_release(cloader_t *c)
{
	if (c->ops.release) {
		if (c->cfg1.data) {
			c->ops.release(c->cfg1.data);
		}
		if (c->cfg2.data) {
			c->ops.release(c->cfg2.data);
		}
	}
	c->is_load_called = 0;
	c->reloader_tid = 0;
}

int cloader_load(cloader_t *c, void *arg)
{
	if (c->is_load_called) {
		return -1;
	}

	c->cfg1.data = c->ops.load(c, arg);
	c->cfg2.data = NULL;
	c->cur = &c->cfg1;
	c->reload = 0;
	c->ref_count = 0;
	c->is_load_called = 1;

	return 0;
}

static inline void pending_empty_ref_count(cloader_t *c)
{
	do {
		if (c->ref_count == 0) {
			break;
		} else {
			cloader_not_busy_wait();
		}
	} while (1);
}

static inline void pending_reload_completed(cloader_t *c)
{
	do {
		if (c->reload == 0) {
			break;
		} else {
			cloader_not_busy_wait();
		}
	} while (1);
}

static inline cloader_cfg_t *next_cloader_cfg(cloader_t *c)
{
	return (c->cur == &c->cfg1) ? &c->cfg2 : &c->cfg1;
}

int cloader_reload(cloader_t *c, void *arg)
{
	cloader_cfg_t *next = NULL;

	if (c->reloader_tid == 0) {
		c->reloader_tid = pthread_self();
	} else {
		assert(c->reloader_tid == pthread_self());
	}

	next = next_cloader_cfg(c);

	assert(next != NULL);

	if (next->data) {
		if (c->ops.release) {
			c->ops.release(next->data);
		}
	}

	// config data loading time too long.
	// so here config data is loaded.
	next->data = c->ops.load(c, arg);
	printf("internal loading completed.\n");


	c->reload = 1;
	{
		pending_empty_ref_count(c);

		// change current config
		c->cur = next;

		if (c->cur == &c->cfg1) {
			printf("reload OK! current is #1\n");
		} else 
		if (c->cur == &c->cfg2){
			printf("reload OK! current is #2\n");
		} else {
			assert("reload OK! current ????");
		}
	}
	c->reload = 0;

	return 0;
}

void cloader_ref_up(cloader_t *c, void **data)
{
	// wait for complete reload
	pending_reload_completed(c);

	atomic_inc(c->ref_count);

	*data = c->cur->data;
}

void cloader_ref_down(cloader_t *c)
{
	atomic_dec(c->ref_count);
}

void cloader_trace(cloader_t *c)
{
	if (c->cur == &c->cfg1) {
		printf("trace: current is #1, ref_count:%u, reload:%d", c->ref_count, c->reload);
	} else 
	if (c->cur == &c->cfg2){
		printf("trace: current is #2, ref_count:%u, reload:%d", c->ref_count, c->reload);
	} else {
		assert("trace: current ????");
	}
}










#define _PACKED_ __attribute__((packed))

typedef struct as_prf_entry {
	char    rbl_addr[128];
	uint8_t rbl_use;
	uint8_t rbl_direct;
	uint8_t rbl_action;
} _PACKED_ as_prf_entry;


typedef struct as_prf_list {
	as_prf_entry       *entry;
	struct as_prf_list *next;
} _PACKED_ as_prf_list;

static void as_release(void *prf)
{
	struct as_prf_list *cur;
	struct as_prf_list *tmp;
	struct as_prf_list *head;
		
	head = (struct as_prf_list*)prf;
	if (!head) {
		return;
	}
	for (cur = head; cur; ) {
		tmp = cur->next;
		if (cur->entry) {
			free(cur->entry);
		}
		free(cur);
		cur = tmp;
	}
	return;
}

static void* as_load(cloader_t *c, void *arg)
{
	static int test_ref = 0;
	int i;
	as_prf_list *head = NULL;
	as_prf_list *cur;
	as_prf_entry *entry;

	for (i=0; i<10; i++) {
		cur = calloc(1, sizeof(*cur));
		{
			entry = calloc(1, sizeof(*entry));
			snprintf(entry->rbl_addr, sizeof(entry->rbl_addr), "%s", "127.0.0.1");
			entry->rbl_use    = test_ref;
			entry->rbl_direct = test_ref;
			entry->rbl_action = test_ref;
			test_ref++;
		}
		cur->entry = entry;

		if (head == NULL) {
			head = cur;
		} else {
			cur->next = head;
			head = cur;
		}
	}

	if (c->is_load_called == 1) {
		usleep(1000);
		printf("!!! reload complete !!!\n");
	}
	return head;
}

static void as_print(as_prf_list *list)
{
	int n = 1;
	as_prf_list *cur;
	as_prf_entry *entry;

	for (cur = list; cur; cur = cur->next) {
		entry = cur->entry;
		entry->rbl_use += n;
//		printf("[#%03d] %s:%d:%d:%d\n", n++, entry->rbl_addr, entry->rbl_use, entry->rbl_direct, entry->rbl_action);
	}
}

static uint32_t called_reader = 1;
void* reader_thread_func(void *arg)
{
	cloader_t *c = (cloader_t*)arg;
	as_prf_list *apl;
	while (1) {
		cloader_ref_up(c, (void **)&apl);
		{
			as_print(apl);

			atomic_inc(called_reader);
			if (called_reader % 5000000 == 0) {
				printf("called_reader = %u\n", called_reader);
				cloader_trace(c);
			}
		}
		cloader_ref_down(c);
	}
	return NULL;
}

void* reloader_thread_func(void *arg)
{
	cloader_t *c = (cloader_t*)arg;
	while (1) {
		cloader_reload(c, NULL);
	}
	return NULL;
}

int main()
{
#define READER_CNT	1
#define LOADER_CNT	1

	cloader_ops_t ops;
	cloader_t loader;
	as_prf_list *apl;
	int i;
	pthread_t reader_tid[READER_CNT];
	pthread_t loader_tid[LOADER_CNT];

	ops.load    = as_load;
	ops.release = as_release;

	cloader_init(&loader, &ops);
	cloader_load(&loader, NULL);

	for (i=0; i<READER_CNT; i++) {
		pthread_create(&reader_tid[i], NULL, reader_thread_func, &loader);
	}
	for (i=0; i<LOADER_CNT; i++) {
		pthread_create(&loader_tid[i], NULL, reloader_thread_func, &loader);
	}

	for (i=0; i<READER_CNT; i++) {
		pthread_join(reader_tid[i], NULL);
	}
	for (i=0; i<LOADER_CNT; i++) {
		pthread_join(loader_tid[i], NULL);
	}
	cloader_release(&loader);
}

