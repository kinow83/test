/*
 * IOUtils.cpp
 *
 *  Created on: 2016. 9. 23.
 *      Author: root
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "IOUtils.h"

namespace kinow {

int IOUtils::extractContent(const char* filename, char *buf, size_t bufsize) {
	int fd;
	int readBytes;

	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		return -1;
	}
	readBytes = read(fd, buf, bufsize);
	close(fd);

	if (readBytes < 0 || readBytes > bufsize) {
		return -1;
	}
	buf[readBytes-1] = '\0';
	return readBytes;
}

int IOUtils::recordContent(const char* filename, char *buf) {
	int fd;
	int writeBytes;
	int buflen;

	if (buf == NULL || buf[0] == '\0') {
		return -1;
	}
	buflen = strlen(buf);

	fd = open(filename, O_CREAT|O_RDWR|O_TRUNC, 644);
	if (fd == -1) {
		return -1;
	}
	writeBytes = write(fd, buf, buflen);
	close(fd);

	if (writeBytes < 0 || writeBytes != buflen) {
		// remove file when failed to write content.
		remove(filename);
		return -1;
	}
	return writeBytes;
}

} /* namespace kinow */
