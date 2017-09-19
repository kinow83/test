#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include "defined.h"


struct Dummy {
	char *name_ptr;
	int int_val;
	char char_val;
};

int main()
{
	printf("[test] defined\n");
	printf("%s\n", __location__);


	printf("[test] __typeof__\n");
	struct Dummy d;
	struct Dummy *ptr_d;

	printf("sizeof(struct Dummy)   = %ld\n", sizeof(__typeof__(d)));
	printf("sizeof(struct Dummy *) = %ld\n", sizeof(__typeof__(ptr_d)));

	char *ptr_c;
	printf("pointer variable sizeof = %ld\n", sizeof(ptr_c));


	const char *const_ptr_c = "kaka__123456";
	printf("const sizeof = %ld\n", sizeof(const_ptr_c));
	printf("* const sizeof = %ld\n", sizeof(*(const_ptr_c)));

	// gcc -D_GNU_SOURCE
	printf("[test] asprintf\n");
	char *as_ptr = NULL;
	asprintf(&as_ptr, "%s created by asprintf\n", "kinow");
	printf("%s -> strlen(%ld)\n", as_ptr, strlen(as_ptr));

	printf("Dummy offset of name_ptr = %ld\n", offsetof(struct Dummy, name_ptr));
	printf("Dummy offset of name_ptr = %ld\n", offsetof(struct Dummy, int_val));
	printf("Dummy offset of name_ptr = %ld\n", offsetof(struct Dummy, char_val));
}
