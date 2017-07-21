#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	struct protoent *pt;
	char *up;
	char *tmp;
	int i;
	for(i=0; i<256; i++) {
		pt = getprotobynumber(i);
		if (pt && pt->p_name) {
			up = strdup(pt->p_name);
			tmp = up;
			for (tmp = up; *tmp; tmp++) {
				*tmp = toupper(*tmp);
			}
			printf("%d is %s\n", i, up);
			free(up);
		} else {
//			printf("%d is null\n", i);
		}
	}

	return 0;
}
