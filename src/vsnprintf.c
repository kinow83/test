/*
 * vsnprintf.c
 *
 *  Created on: 2017. 6. 3.
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>

int main()
{
	int needs;

	needs = snprintf(NULL, 0, "%d", 1);
	printf("need: %d\n", needs);
	return 0;
}
