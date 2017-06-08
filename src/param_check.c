/*
 * param_check.c
 *
 *  Created on: 2017. 6. 3.
 *      Author: root
 */

#include <stdio.h>
#include <stdarg.h>

/*
    __attribute__((format(scanf,m,n)))
( 여기서 m은 format string이 시작되는 위치, n은 첫번째 가변 파라메타의 위치이다.)
 */
void kprintf(char *fmt, ...) __attribute__((format(printf, 1, 2)));

void kprintf(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

int main()
{
	printf("%d, %d, %d\n", 1, 2);
/*
param_check.c: In function ‘main’:
param_check.c:21:9: warning: format ‘%d’ expects a matching ‘int’ argument [-Wformat=]
printf("%d, %d, %d\n", 1, 2);
 */


	kprintf("%d, %d, %d\n", 1, 2);


	return 0;
}
