#include "shash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "sampling.h"

int _compare(const void *a, const void *b)
{
	const char *sa = (const char *)a;
	const char *sb = (const char *)b;
	return strcmp(sa, sb);
}

uint16_t _hash(const void *h)
{
// http://www.cse.yorku.ca/~oz/hash.html

	// djb2
	uint16_t hash = 5381;
	int c;
	char *str = (char *)h;

	while (c = *str++) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash;

#if 0
	// sdbm
	unsigned long hash = 0;
	int c;

	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
#endif
}

void _freebucket(void *b)
{
	free(b);
}


int main(int argc, char** argv)
{
	hash_table_t *h;
	size_t num = 0;
	struct word *w = NULL;
	struct word *c = NULL;
	struct word *f = NULL;
	clock_t start;
//	hash_bucket_t *b;

	num = load_word(argc, argv, 20, 20, &w);
	num = load_word(argc, argv, 20, 20, &f);

	h = hash_table_create(_compare, _hash, _freebucket, 1);

	printf("num = %ld\n", num);

	printf("shash\n");
	start = clock();
	for (c=w; c; c=c->next) {
		hash_table_insert(h, c->text);
	}
	printf("shash insert: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

	start = clock();
	for (c=f; c; c=c->next) {
		assert (hash_table_find(h, c->text) != NULL);
	}
	printf("shash find: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);


	//hash_table_free(h);
}

