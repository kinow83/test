#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "libsample.h"

uint32_t gen_magic(char *str)
{
	uint32_t m = 0;
	int i, n =strlen(str);
	for (i=0; i<n; i++) {
		m ^= m * 31;
	}
	return m;
}

struct word * get_word(struct word *head, size_t n, size_t total)
{
	size_t i;

	if (n > total) return NULL;

	for (i=0; i<n; i++) {
		head = head->next;
	}
	return head;
}

static
size_t load_word_per_file(char *file, size_t minlen, size_t maxlen, struct word **ppw)
{
	FILE *fp;
	size_t num = 0;
	struct word *w, *h = NULL, *l = NULL;
	char buf[1024] = {0, };
	size_t len;
	size_t rlen, i;

	srand(time(NULL));
	printf("%s\n", file);

	fp = fopen(file, "r");
	if (!fp) return 0;

	if (minlen == maxlen) {
		rlen = (rand() % (maxlen - minlen + 1)) + minlen;
	} else {
		rlen = minlen;
	}

	while ((len = fread(buf, 1, rlen, fp)) > 0) {
		if (len !=rlen)
			continue;

		if (strstr(buf, "\n")) 
			continue;
		
		w = calloc(1, sizeof(*w));
		snprintf(w->text, sizeof(w->text), "%s", buf);
		w->magic = gen_magic(buf);
		if (!h) {
			l = h = w;
		} else {
			w->next = h;
			h = w;
		}
		num++;
//		printf("%s\n", w->text);

		if (minlen == maxlen) {
			rlen = (rand() % (maxlen - minlen + 1)) + minlen;
		} else {
			rlen = minlen;
		}

		memset(buf, 0, sizeof(buf));
	}

	if (*ppw == NULL)
		*ppw = h;
	else {
		l->next = *ppw;
		*ppw = h;
	}

DONE:
	fclose(fp);
	return num;
}

size_t load_word(int argc, char **argv, size_t minlen, size_t maxlen, struct word **ppw)
{
	size_t num = 0;
	int i;

	for (i=1; i<argc; i++) {
		num += load_word_per_file(argv[i], minlen, maxlen, ppw);
	}
//	printf("total = %u\n", num);
	return num;
}


