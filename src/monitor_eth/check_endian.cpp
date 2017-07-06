#include <stdio.h>
#include <stdint.h>
#include <endian.h>

main()
{
	union {
    	int i;
	    char c[sizeof(int)];
	} x;

	x.i = 1;
	if(x.c[0] == 1)
    	printf("little-endian\n");
	else
		printf("big-endian\n");

	printf("%d\n", 	__BYTE_ORDER);
	printf("%d\n", __BIG_ENDIAN);
	printf("%d\n", 	__LITTLE_ENDIAN);
}
