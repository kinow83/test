#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/unistd.h>
#include <pthread.h>

void *thread_fn(void *arg);

void print_ids(const char *s) {
	pid_t pid;
	pthread_t tid;

	pid = getpid();
	tid = pthread_self();

	printf("%s : pid=%u, pthread_self()=%u, syscall(__NR_gettid)=%ld\n", s,
		(unsigned int)pid, (unsigned int)tid, syscall(__NR_gettid));
}

void *thread_fn(void *arg) {
	print_ids("new  thread");
	sleep(10000);
	return 0;
}

int main()
{
	int rc;
	pthread_t tid;
	int status;

	rc = pthread_create(&tid, NULL, thread_fn, NULL);
	if (rc) {
		perror("thread_create()");
		return -1;
	}
	print_ids("main thread");
	sleep(1);

	system("ps -efL |grep pthread_id |grep -v grep");

	getchar();

	pthread_join(tid, (void**)&status);

	return 0;
}

/*
kinow-mint pthread # ./pthread_id 

main thread : pid=26717, pthread_self()=2036000512, syscall(__NR_gettid)=26717
new  thread : pid=26717, pthread_self()=2027734784, syscall(__NR_gettid)=26718

UID        PID  PPID   LWP  C NLWP STIME TTY          TIME CMD
root     26717 23575 26717  0    2 17:29 pts/8    00:00:00 ./pthread_id
root     26717 23575 26718  0    2 17:29 pts/8    00:00:00 ./pthread_id
*/


