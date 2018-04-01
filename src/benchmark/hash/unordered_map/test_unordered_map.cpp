#include <iostream>
#include <unordered_map>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "libsample.h"

using std::string;

struct char_hash
{
	std::size_t operator()(const char* m) const  {
		size_t len = strlen(m);
		size_t hash = 0;
		for (int i=0; i<len; i++) {
			hash ^= m[i];
		}
		return hash;
	}
};

struct char_compare {
	bool operator()(const char *a, const char *b) const {
		size_t alen = strlen(a);
		size_t blen = strlen(b);
		if (alen != blen) return false;
		return strncmp(a, b, alen) == 0 ? true : false;
	}
};


int main(int argc, char **argv)
{
	size_t num = 0;
	struct word *w = NULL;
	struct word *c = NULL;
	struct word *f = NULL;
	std::unordered_map<char *, char *, char_hash, char_compare> mymap;
	clock_t start;
	char *s;

	num = load_word(argc, argv, 20, 20, &w);
	num = load_word(argc, argv, 20, 20, &f);
	printf("num = %ld\n", num);

	printf("unorderd_map (char*) in libstdc++\n");
	start = clock();
	for (c=w; c; c=c->next) {
		mymap[c->text] = c->text;
	}
	printf("unordered insert: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

	int cnt = 0;
	start = clock();
	for (c=f; c; c=c->next) {
		s = mymap[c->text];
//		printf("%p %p (%s, %s)\n", c->text, s, c->text, s);
		assert (s != NULL);
	}
	printf("unordered find: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

	printf("\n");
}
