/*
 * user_ioctl.c
 *
 *  Created on: 2016. 10. 14.
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#define DEV_NAME "/dev/virtual_device"
#define MY_IOCTL_NUMBER 100

#define MY_IOC_READ      _IOR( MY_IOCTL_NUMBER, 0, int )
#define MY_IOC_WRITE     _IOW( MY_IOCTL_NUMBER, 1, int )
#define MY_IOC_STATUS    _IO ( MY_IOCTL_NUMBER, 2)
#define MY_IOC_READWRITE _IOWR(MY_IOCTL_NUMBER, 3, int)
#define MY_IOC_NUMBER    4


int main(int argc, char** argv)
{
	int fd;
	int data = 100;

	fd = open(DEV_NAME, O_RDWR);
	if (fd < 0) {
		printf("error - %s (%s)\n", strerror(errno), DEV_NAME);
		exit(-1);
	}

	printf("\n---------------------\n");
	printf("before read : %d\n", data);
	if (ioctl( fd, MY_IOC_READ, &data ) < 0) {
		perror("ioctl MY_IOC_READ");
		close(fd);
		exit (-1);
	}
	printf("after  read : %d\n", data);

	printf("\n---------------------\n");
	data = 200;
	printf("before write : %d\n", data);
	if (ioctl( fd, MY_IOC_WRITE, &data ) < 0) {
		perror("ioctl MY_IOC_WRITE");
		close(fd);
		exit (-1);
	}
	if (ioctl( fd, MY_IOC_READ, &data ) < 0) {
		perror("ioctl MY_IOC_READ");
		close(fd);
		exit (-1);
	}
	printf("after  write : %d\n", data);

	printf("\n---------------------\n");
	data = 400;
	printf("before read write : %d\n", data);
	if (ioctl( fd, MY_IOC_READWRITE, &data ) < 0) {
		perror("ioctl MY_IOC_READWRITE");
		close(fd);
		exit (-1);
	}
	printf("after  read write : %d\n", data);

	close(fd);

	return 0;
}
