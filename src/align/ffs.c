#include <strings.h>
#include <stdio.h>


int main()
{
	unsigned x;

	x = 0x10000000U;
	printf("%d\n", ffs(x));

	x = 0x08000000U;
	printf("%d\n", ffs(x));

	x = 0x01000000U;
	printf("%d\n", ffs(x));

	x = 0x80000000U;
	printf("%d\n", ffs(x));
}
