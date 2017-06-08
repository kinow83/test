/*
 * test.c
 *
 *  Created on: 2017. 6. 3.
 *      Author: root
 */


#include <stdio.h>
#include <unistd.h>

struct A {
	int c;
	int d;
};

int main()
{

	struct A a = {
			.c = 1,
			.d = 2,
	};
	struct A *pa = &a;
	char *pp = NULL;

	printf("%p\n", &((typeof(pa))NULL)->c);
	printf("%p\n", &((typeof(pa))NULL)->d);

	return 0;
}
