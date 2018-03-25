#include <iostream>
#include <unordered_map>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "sampling.h"



int main(int argc, char **argv)
{
	size_t i;
	size_t num = 0;
	struct word *w = NULL, *c;
//	std::string f;
	char *f;
	std::unordered_map<char*, char*> mymap;

	num = load_word(argc, argv, 20, 20, &w);
	printf("num = %ld\n", num);

	i = 0;
	for (c=w; c; c=c->next) {
		mymap[c->text] = c->text;
	}
	printf("completed add\n");

#if 1
	clock_t start = clock();
	i = 0;
	for (c=w; c; c=c->next) {
//		if (i++ % 10240 == 0) 
//			printf("process find: %d\n", i);

		f = mymap[c->text];
		assert( f != NULL );
//		printf("%s = %s\n", c->text, f);
	}
	printf("%0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

#endif
}
