
/*
 * ioctl.c
 *
 *  Created on: 2016. 10. 5.
 *      Author: root
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> /* open */
#include <unistd.h> /* exit */
#include <sys/ioctl.h> /* ioctl */

void main(int argc, char** argv)
{
	int fd;

	if (argc != 2) {
		fprintf(stderr, "Usage: filename\n");
		exit(-1);
	}

	fd = open(argv[1], 0);
	if (fd < 0) {
		perror("open - ");
		exit(-1);
	}
	close(fd);
}
