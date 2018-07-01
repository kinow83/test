#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main()
{
	int i=0;

	while (1) {

		char *p = malloc(1024*1024);
		usleep(10000);
		printf(".\n");
	}
	return 0;
}
