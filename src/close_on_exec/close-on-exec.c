/*
 * close-on-exec.c
 *
 *  Created on: 2017. 9. 16.
 *      Author: root
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {
	int fd;
	int val;
	char cmd[256];

	snprintf(cmd, sizeof(cmd), "%s/loop.sh", getenv("PWD"));

	fd = open("exec_copy.txt", O_CREAT);

	// FD_CLOEXEC 값을 fcntl 을 이용해서
	// 가져온 다음 세팅되어 있는지 검사한다.
	val = fcntl(fd, F_GETFD, 0);
	if (val & FD_CLOEXEC)
		printf("close-on-exec bit on\n");
	else
		printf("close-on-exec bit off\n");

	// FD_CLOEXEC 를 세팅한다.
	val |= FD_CLOEXEC;
	if (val & FD_CLOEXEC)
		printf("close-on-exec bit on\n");
	else
		printf("close-on-exec bit off\n");
	fcntl(fd, F_SETFD, val);

	// loop 프로그램을 exec 시킨다.
	execl(cmd, "./loop.sh", NULL);
}

/*

mint-dev fd # ls -al
total 0
dr-x------ 2 root root  0  9월 16 15:39 .
dr-xr-xr-x 9 root root  0  9월 16 15:39 ..
lrwx------ 1 root root 64  9월 16 15:39 0 -> /dev/pts/2
lrwx------ 1 root root 64  9월 16 15:39 1 -> /dev/pts/2
lrwx------ 1 root root 64  9월 16 15:39 2 -> /dev/pts/2
lr-x------ 1 root root 64  9월 16 15:39 255 -> /root/git/libs/close_on_exec/loop.sh

 */
