#include <iostream>
#include <unordered_map>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "libsample.h"

using std::string;

int main(int argc, char **argv)
{
	size_t num = 0;
	struct word *w = NULL;
	struct word *c = NULL;
	struct word *f = NULL;
	std::unordered_map<string, string> mymap;
	clock_t start;
	string s;

	num = load_word(argc, argv, 20, 20, &w);
	num = load_word(argc, argv, 20, 20, &f);
	printf("num = %ld\n", num);

	printf("unorderd_map (string) in libstdc++\n");
	start = clock();
	for (c=w; c; c=c->next) {
		mymap[string(c->text)] = string(c->text);
	}
	printf("unordered insert: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

	int cnt = 0;
	start = clock();
	for (c=f; c; c=c->next) {
		s = mymap[string(c->text)];
		assert (s.empty() != true);
	}
	printf("unordered find: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

	printf("\n");
}
