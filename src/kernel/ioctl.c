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

#include "chardev.h"


void ioctl_set_msg(int fd, char *message)
{
	int ret;

	printf("IOCTL_SET_MSG \t= %ld\n", IOCTL_SET_MSG);
	printf("IOCTL_GET_MSG \t= %ld\n", IOCTL_GET_MSG);
	printf("IOCTL_GET_NTH_BYTE \t= %ld\n", IOCTL_GET_NTH_BYTE);

	ret = ioctl(fd, IOCTL_SET_MSG, message);
	if (ret < 0) {
		printf("failed to ioctl_set_msg: %d\n", ret);
		exit(-1);
	}
}

void ioctl_get_msg(int fd)
{
	int ret;
	char message[100];

	ret = ioctl(fd, IOCTL_GET_MSG, message);
	if (ret < 0) {
		printf("failed to ioctl_get_msg: %d\n", ret);
		exit(-1);
	}
	printf("get_msg message: %s\n", message);
}

void ioctl_get_nth_byte(int fd)
{
	char c;
	int i = 0;

	printf("get_nth_byte message: ");

	do {
		c = ioctl(fd, IOCTL_GET_NTH_BYTE, i++);
		if (c < 0) {
			printf("failed to ioctl_get_nth_byte at the %d'th byte:", i);
			exit(-1);
		}
		putchar(c);

	} while (c != 0);

	putchar('\n');
}

void main()
{
	int fd;
	int ret;
	char *msg = "Message passed by ioctl\n";

	fd = open(DEVICE_NAME, 0);
	if (fd < 0) {
		printf("Can't open device file: %s\n", DEVICE_NAME);
	}

	ioctl_set_msg(fd, msg);
	ioctl_get_nth_byte(fd);
	ioctl_get_msg(fd);
}
