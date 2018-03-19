#include <search.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "kentry.h"

static void *root = NULL;

static int
compare(const void *pa, const void *pb)
{
	KENTRY *ka = (KENTRY *)pa;
	KENTRY *kb = (KENTRY *)pb;

	if (ka->magic < kb->magic)
		return -1;
	if (ka->magic > kb->magic)
		return 1;
	if (ka->len < kb->len)
		return -1;
	if (ka->len > kb->len)
		return 1;
	return strcmp(ka->value, kb->value);
}

static void
action(const void *nodep, const VISIT which, const int depth)
{
	int *datap;

	switch (which) {
		case preorder:
			break;
		case postorder:
			datap = *(int **) nodep;
			printf("%6d\n", *datap);
			break;
		case endorder:
			break;
		case leaf:
			datap = *(int **) nodep;
			printf("%6d\n", *datap);
			break;
	}
}

static uint32_t 
hash32(const char *str)
{
	int c;
	uint32_t hash = 0;
	while (c = *str++) {
		hash += c;
	}
	return hash;
}

int
main(int argc, char **argv)
{
	KENTRY *entry;
	KENTRY search;
	char buf[1024] = {0, };
	FILE *samplefp;
	FILE *searchfp;
	int nsample = 0;
	int nsearch = 0;

	samplefp = fopen(argv[1], "r");
	searchfp = fopen(argv[2], "r");

	while (fgets(buf, sizeof(buf), samplefp) > 0) {
		if (buf[0] == '#') continue;
		buf[strlen(buf)-1] = 0;
		entry = malloc(sizeof(*entry));
		entry->magic = hash32(buf);
		entry->len = snprintf(entry->value, sizeof(entry->value), "%s", buf);
		tsearch((void *)entry, &root, compare);
//		printf("add %s\n", entry->value);
		nsample++;
	}

	getchar();

	while (fgets(buf, sizeof(buf), searchfp) > 0) {
		buf[strlen(buf)-1] = 0;
		search.magic = hash32(buf);
		search.len = snprintf(search.value, sizeof(search.value), "%s", buf);
		entry = tfind((void *)&search, &root, compare);
		if (entry) {
//			printf("find %s\n", (*(KENTRY**)entry)->value);
			nsearch++;
		}
	}

	printf("matched: %d/%d\n", nsearch, nsample);
	fclose(samplefp);
	fclose(searchfp);
	tdestroy(root, free);
	exit(EXIT_SUCCESS);
}

