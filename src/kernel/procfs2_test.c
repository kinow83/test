/*
 * procfs2_test.c
 *
 *  Created on: 2016. 10. 6.
 *      Author: root
 */




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> /* open */
#include <unistd.h> /* exit */

#define PROCFS_BUFFER1K "/proc/buffer1k"

int main(int argc, char** argv)
{
	int fd;
	char *buffer;
	int read_byte;

	if (argc != 2) {
		fprintf(stderr, "Usage: read size\n");
		exit(-1);
	}

	read_byte = atoi(argv[1]);
	buffer = (char *)malloc(read_byte);
	memset(buffer, 0, read_byte);

	fd = open(PROCFS_BUFFER1K, 0);
	if (fd < 0) {
		perror("open - ");
		exit(-1);
	}

	if (read(fd, buffer, read_byte) < 0) {
		perror("read - ");
		close(fd);
		exit(-1);
	}

	printf("%s\n", buffer);

	close(fd);

	return 0;
}
