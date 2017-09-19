/*
 * format.c
 *
 *  Created on: 2016. 11. 6.
 *      Author: root
 */

#include <stdio.h>

/* like printf() but to standard error only */
extern void eprintf(const char *format, ...)
	__attribute__((format(printf, 1, 2))) /* 1=format 2=params */
;

/* printf only if debugging is at the desired level */
extern void dprintf(int dlevel, const char *format, ...)
	__attribute__((format(printf, 2, 3)))  /* 2=format 3=params */
;


void eprintf(const char *format, ...) {
	printf("call eprintf\n");
}

int main(int argc, char **argv)
{
	eprintf("s=%s\n", "lala");
	eprintf("s=%s\n", 5);          // error(warning)
	eprintf("n=%d,%d,%d\n", 1, 2); // error(warning)
}

/*
 * __attribute__((format(printf,m,n)))
 * __attribute__((format(scanf ,m,n)))
 * The (m) is the number of the "format string" parameter,
 * and (n) is the number of the first variadic parameter.
 */
