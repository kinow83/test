/*
 * open-on-exec.c
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

	printf("%s\n", cmd);

	execl(cmd, "./loop.sh", NULL);
}

/*
mint-dev fd # ls -al
total 0
dr-x------ 2 root root  0  9월 16 15:36 .
dr-xr-xr-x 9 root root  0  9월 16 15:36 ..
lrwx------ 1 root root 64  9월 16 15:37 0 -> /dev/pts/2
lrwx------ 1 root root 64  9월 16 15:37 1 -> /dev/pts/2
lrwx------ 1 root root 64  9월 16 15:36 2 -> /dev/pts/2
lr-x------ 1 root root 64  9월 16 15:37 255 -> /root/git/libs/close_on_exec/loop.sh
lr-x------ 1 root root 64  9월 16 15:37 3 -> /root/git/libs/close_on_exec/exec_copy.txt
 */
