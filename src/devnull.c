#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



int main()
{
	
	pid_t fs;


	if (getppid() == 1) {
		printf("my parent is init process\n");
		return -1;
	}

	fs = fork();
	if (fs > 0) {
		sleep(2);
		exit(0);
	}
	if (fs < 0) {
		perror("fork();");
		exit(1);
	}
	setsid();

	close(0);
	close(1);
	close(2);

	printf("aaaaaaaaaa\n");

	char cmd[256];
	int fd1, fd2;
	
	fd1 = open("/dev/null", O_RDWR);
	sprintf(cmd, "echo 'fd1 = %d' > /tmp/kaka", fd1);
	system(cmd);

	fd2 = open("/dev/null", O_RDWR);
	sprintf(cmd, "echo 'fd2 = %d' >> /tmp/kaka", fd2);
	system(cmd);

	return 0;
}
