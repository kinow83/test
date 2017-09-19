/*
 * STDLib.cpp
 *
 *  Created on: 2016. 8. 3.
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "STDLib.h"

namespace kinow {

void STDLib::closeAllFileDescriptions() {
	int fd = 0;
	int max = getdtablesize();
	for (fd = 0; fd < max; fd++) {
		close(fd);
	}
}

int STDLib::safeSystem(const char* cmd) {
	int pid = 0;
	int status = 0;

	if (cmd == 0) {
		return 1;
	}

	pid = fork();
	if (pid == -1) {
		return -1;
	}
	if (pid == 0) {
		const char *argv[4];
		argv[0] = "bash";
		argv[1] = "-c";
		argv[2] = cmd;
		argv[3] = 0;
		close_all_fd();
		execve("/bin/bash", (char **) argv, environ);
		exit(127);
	}
	do {
		if (waitpid(pid, &status, 0) == -1) {
			if (errno != EINTR) {
				return -1;
			}
		} else {
			return status;
		}
	} while (1);

	return 0;
}

} /* namespace kinow */
