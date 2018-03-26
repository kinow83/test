#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


uint64_t allocated = 0;

void *memalloc(size_t siz)
{
	void *_new;
	allocated += siz;
	_new = malloc(siz + sizeof(uint64_t));
	*(uint64_t*)_new = (uint64_t)siz;
	return (char *)_new + sizeof(uint64_t);
}

void memfree(void *p)
{
	void *org = (char *)p - sizeof(uint64_t);
	uint64_t siz = *(uint64_t *)org;
	allocated -= siz;
	free(org);
}

typedef struct tnode {
	struct tnode *child[26];
	void *data;
} tnode_t;


int main(int argc, char **argv)
{
	char *str[10];
	int i;
	for (i=0; i<10; i++) {
		str[i] = memalloc(100);
		snprintf(str[i], 10, "%d", i);
	}

	printf("%lu\n", allocated);

	for (i=0; i<10; i++) {
		memfree(str[i]);
	}
	printf("%lu\n", allocated);
}
