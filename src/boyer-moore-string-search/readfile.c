/*
 * readfile.c
 *
 *  Created on: 2017. 8. 11.
 *      Author: root
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "readfile.h"

char *readfile(const char *filename)
{
	int fd;
	char tmp[1024];
	char *text, *p;
	size_t textlen, len;
	struct stat st;

	if (stat(filename, &st) == -1) {
		perror("stat: ");
		exit(1);
	}

	textlen = st.st_size;
	text = (char *) malloc(textlen + 1);
	text[textlen] = '\0';

	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		perror("filename: ");
		exit(1);
	}

	p = text;
	while ((len = read(fd, tmp, sizeof(tmp))) != 0) {
		memcpy(p, tmp, len);
		p += len;
	}
	close(fd);
	return text;
}

#if 0

int main(int argc, char **argv)
{
	const char *filename;
	char *text;

	if (argc != 2) {
		fprintf(stderr, "[filename]\n");
		exit(1);
	}
	filename = argv[1];

	text = readfile(filename);
	printf("%s\n", text);

	free(text);
}

#endif
