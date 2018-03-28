#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>

#include "extract.h"

size_t readfile(const char *file, char *buf, size_t buflen)
{
	int fd;

	fd = open(file, O_RDONLY);
	if (read(fd, buf, buflen) != buflen) {
		close(fd);
		return 0;
	}
	close(fd);
	return buflen;
}

char *rseek(char *buf, size_t pos, size_t len)
{
	char *_new = malloc(len+1);
	memcpy(_new, buf+pos, len);
	_new[len] = 0;
	return _new;
}



#if 0
/*

0            50            100
|            |             |
       |             |

*/

int main(int argc, char **argv)
{
	size_t len;
	char *buf;
	size_t buflen = 1024*1024*20;
	size_t pos;

	buf = malloc(buflen);

	assert (readfile(argv[1], buf, buflen) != 0);

	size_t chunk = (buflen / 8);
	int i;
	char *seek[8];
	len = chunk/2 > 20 ? 20 : chunk/2;
	for (i=0; i<8; i++) {
		seek[i] = rseek(buf, chunk * i, len);
		printf("%ld: %s\n", strlen(seek[i]), seek[i]);
	}
}
#endif
