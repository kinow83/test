#define _GNU_SOURCE
#include <search.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "sampling.h"


typedef struct table_t {
	struct hsearch_data htab;
	size_t size;
} table_t;


table_t* table_create(size_t size)
{
	table_t* table = calloc(1, sizeof(*table));

	hcreate_r(size, &table->htab);

	return table;
}

void table_destroy(table_t* table)
{
	hdestroy_r(&table->htab);
	free(table);
	table = NULL;
}

int table_add(table_t* table, char* key, void* data)
{
	unsigned n = 0;
	ENTRY e, *ep;

	e.key = key;
	e.data = data;
	n = hsearch_r(e, ENTER, &ep, &table->htab);

	return n;
}

void* table_get(table_t* table, char* key)
{
	unsigned n = 0;
	ENTRY e, *ep;

	e.key = key;
	n = hsearch_r(e, FIND, &ep, &table->htab);

	if (!n)
		return NULL;

	return ep->data;
}

int main(int argc, char* argv[])
{
	size_t num = 0;
	struct word *w = NULL;
	struct word *c = NULL;
	struct word *f = NULL;
	clock_t start;

	num = load_word(argc, argv, 20, 20, &w);
	num = load_word(argc, argv, 20, 20, &f);
	printf("num = %ld\n", num);

	table_t* table = table_create(num);

	printf("gnu hsearch\n");
	start = clock();
	for (c=w; c; c=c->next) {
		table_add(table, c->text, c->text);
	}
	printf("gnu hsearch insert: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);


	start = clock();
	for (c=f; c; c=c->next) {
		assert(table_get(table, c->text) != NULL);
	}
	printf("gnu hsearch: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

	table_destroy(table);

	printf("\n");

	return 0;
}

#if 0
int main(int argc, char* argv[])
{
	unsigned i = 0;
	int n = atoi(argv[1]);
	table_t* table = table_create(n);
	void* d = NULL;
	size_t num = 0;
	struct word *w = NULL, *c;
	char *f;
	char **arr;

	arr = malloc(sizeof(char *) * n);
	for (i=0; i<n; i++) {
		arr[i] = malloc(10);
		snprintf(arr[i], 10, "%d", i);
		table_add(table, arr[i], arr[i]);
		printf("%s\n", arr[i]);
	}
	for (i=0; i<n; i++) {
		f = table_get(table, arr[i]);
		printf("%p\n", f);
	}
	table_destroy(table);

	return 0;
}
#endif

