#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "mailutils/header.h"
#include "mailutils/stream.h"
#include "mailutils/address.h"
#include "mailutils/message.h"

void *tf(void *data)
{
	char *buf = (char*)data;
	char *pval = NULL;
	char *plang = NULL;
	int i, flags = 0;

	for (i=0; i<10; i++) {
		int	rc = mu_mimehdr_decode_param (buf, flags, "UTF-8", &pval, &plang);
		//	printf("rc=0x%4x\n", rc);
		if (pval) {
			printf("%s : %ld\n", pval, pthread_self());
			usleep(2);
			free(pval);
		}
		if (plang) {
			//		printf("%s\n", plang);
			free(plang);
		}
	}
}

int main(int argc, char **argv)
{
	mu_header_t header;
	int rc;
	char *mbox = argv[1];
	struct stat st;
	char *buf;
	FILE *fp;
	mu_stream_t stream;

	if (stat(mbox, &st)) {
		perror("stat");
		exit(1);
	}
	buf = (char *)malloc(st.st_size + 2);
	fp = fopen(mbox, "r");
	if (!fp) {
		perror("fopen");;
		exit(1);
	}
	rc = fread(buf, 1, st.st_size, fp);
	fclose(fp);
	if (rc != st.st_size) {
		exit(1);
	}


	buf[st.st_size] = '\n';
	buf[st.st_size+1] = 0;
//printf("==============\n");
//printf("%s\n", buf);
//printf("==============\n");

	int i;
	const int ts = 20;
	int status;
	pthread_t t[ts];
	for (i=0; i<ts; i++) {
		pthread_create(&t[i], NULL, tf, (void*)buf);
	}

	for (i=0; i<ts; i++) {
		pthread_join(t[i], (void**)&status);
	}
}
