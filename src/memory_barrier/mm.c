#include <string.h>
#include <unistd.h>
#include "mm.h"

struct ctx CTX[10000];

void copy(void *from , void *to, size_t len)
{
	memcpy(from, to, len);
}

void func(int n)
{
	char inner[50];

	copy(CTX[n].data, inner, 50);

	CTX[n].used = 7;
}

int func2(int c)
{
	return c + 10 / 2 +30;
}
