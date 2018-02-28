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

struct cloader_t;

typedef struct cloader_ops_t {
	void* (*load)(struct cloader_t *, void *);
	void (*release)(void *);
} cloader_ops_t;

typedef struct cloader_t {
	cloader_ops_t ops;
	void *prf_1;
	void *prf_2;
	void *prf_c;
	int reload;
	int ref_count;
} cloader_t;


int init_cloader(cloader_t *c, cloader_ops_t *ops)
{
	memset(c, 0, sizeof(*c));
	c->ops.load = ops->load;
	c->ops.release = ops->release;

	assert((c->ops.load != NULL));
	return 0;
}

void release_cloader(cloader_t *c)
{
	if (c->ops.release) {
		if (c->prf_1) {
			c->ops.release(c->prf_1);
		}
		if (c->prf_2) {
			c->ops.release(c->prf_2);
		}
	}
}

int load_cloader(cloader_t *c, void *arg)
{
	assert(c->ops.load != NULL);

	c->prf_1 = c->ops.load(c, arg);
	c->prf_2 = NULL;
	c->prf_c = c->prf_1;
	c->reload = 0;
	c->ref_count = 0;

	return 0;
}

int reload_cloader(cloader_t *c, void *arg)
{
	void *next_prf = NULL;

	if (c->reload == 1) {
		return 0;
	}

	c->reload = 1;
	while (1) {
		if (c->ref_count == 0) {
			break;
		} else {
			usleep(500);
		}
	}

	if (c->prf_c == c->prf_1) {
		next_prf = c->prf_2;
	} else {
		next_prf = c->prf_1;
	}

	if (next_prf) {
		if (c->ops.release) {
			c->ops.release(next_prf);
		}
	}

	assert(c->ops.load != NULL);
	next_prf = c->ops.load(c, arg);

	c->prf_c = next_prf;
	if (c->prf_c == c->prf_1) {
		printf("current profile is 1\n");
	} else {
		printf("current profile is 2\n");
	}

	c->reload = 0;

	return 0;
}

void get_current_cloader(cloader_t *c, void **pprf)
{
	atomic_inc(c->ref_count);
	if (c->reload == 1) {
		atomic_dec(c->ref_count);
		while (1) {
			if (c->reload == 0) {
				atomic_inc(c->ref_count);
				break;
			} else {
				usleep(100);
			}
		}
	}

	*pprf = c->prf_c;
	atomic_dec(c->ref_count);
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

static void as_print(as_prf_list *list)
{
	int n = 1;
	as_prf_list *cur;
	as_prf_entry *entry;

	for (cur = list; cur; cur = cur->next) {
		entry = cur->entry;
		printf("[#%03d] %s:%d:%d:%d\n", n++, entry->rbl_addr, entry->rbl_use, entry->rbl_direct, entry->rbl_action);
	}
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
	return head;
}

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
		printf("1111111111\n");
		tmp = cur->next;
		if (cur->entry) {
			free(cur->entry);
		}
		free(cur);
		cur = tmp;
	}
	return;
}

int main()
{
	cloader_ops_t ops;
	cloader_t loader;
	as_prf_list *apl;

	ops.load    = as_load;
	ops.release = as_release;

	init_cloader(&loader, &ops);
	printf("done init_cloader\n");

	load_cloader(&loader, NULL);
	printf("done load_cloader\n");
	get_current_cloader(&loader,(void**)&apl);
	as_print(apl);

	release_cloader(&loader);
}


#if 0
cloader_ops_t p;
cloader_t c;

void* handler(void *arg)
{
	int i;
	test_config *t;

	for (i=0; i<10000000; i++) {
		get_current_cloader(&c, (void **)&t);
		printf("1. %d\n" ,t->num);

		reload_cloader(&c, NULL);

		get_current_cloader(&c, (void **)&t);
		printf("2. %d\n" ,t->num);
	}

	return NULL;
}

int main()
{
	int i;
	int N = 16;
	pthread_t tid[N];
	p.load = test_load;
	p.release = NULL;
	init_cloader(&c, &p);

	load_cloader(&c, NULL);

	for (i=0; i<N; i++) {
		pthread_create(&tid[i], NULL, handler, NULL);
	}
	for (i=0; i<N; i++) {
		pthread_join(tid[i], NULL);
	}
	return 0;
}
#endif

