#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

int count;
typedef struct foo {
	int id;
	char name[256];
} foo;

static int n_id = 0;

void alloc()
{
	foo *f = malloc(sizeof(foo));
	f->id = n_id++;
	snprintf(f->name, sizeof(f->name), "%d:%s", f->id, f->name);
	free(f);
}

void *thread_handler(void *arg)
{
	int i;
	for (i=0; i<count; i++) {
		alloc();
	}
}
int main(int argc, char **argv)
{
	count = atoi(argv[1]);
	int i;
	pthread_t tid[10];

	for (i=0; i<10; i++) {
		pthread_create(&tid[i], NULL, thread_handler, NULL);
	}

	for (i=0; i<10; i++) {
		pthread_join(tid[i], NULL);
	}
}
