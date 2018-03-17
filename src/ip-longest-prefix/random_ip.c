#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

uint32_t get_r(int fd)
{
	uint32_t t;
	read(fd, &t, sizeof(t));
	return t & 0xFF;
}

int main(int argc, char **argv)
{
	int n = atoi(argv[1]);
	int i;
	int fd = open("/dev/urandom", O_RDONLY);

	for (i=0; i<n;i++) {
		printf("%d.%d.%d.%d\n", get_r(fd), get_r(fd), get_r(fd), get_r(fd));
	}

	close(fd);
}
