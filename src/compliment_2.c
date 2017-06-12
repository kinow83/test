/*
 * compliment_2.c
 *
 *  Created on: 2017. 6. 12.
 *      Author: root
 */

#include <stdio.h>

#define MAX_ERRNO	4095
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

void compliment(const long *pN, long *pC) {
	*pC = ~(*pN) + 1;
	return;
}

int *arr[] = {
		NULL,
		NULL,
		NULL,
};

int main()
{
	long n = 1, c;

	printf("%ld \t 0x%x\n", n, n);
	printf("%ld \t 0x%x\n", -n, -n);
	printf("%ld \t 0x%x\n", ~n, ~n);

	compliment(&n, &c);
	printf("%ld \t 0x%x\n", c, c);


	printf("MAX_ERRNO: %ld \t 0x%x\n", (unsigned long)-MAX_ERRNO, (unsigned long)-MAX_ERRNO);
	printf("MAX_ERRNO: %ld \t 0x%x\n", (unsigned long)-MAX_ERRNO, -MAX_ERRNO);


	printf("sizeof = %d\n", sizeof(arr) / sizeof(int *));

	return 0;
}
