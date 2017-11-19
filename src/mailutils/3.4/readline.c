#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mailutils/header.h"
#include "mailutils/stream.h"
#include "mailutils/address.h"

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

//	printf("%s\n", buf);

	// last: \n
	rc = mu_header_create(&header, buf, st.st_size+1);
	free(buf);
	if (rc) {
		exit(1);
	}
#if 0
	mu_address_t addr;
	mu_header_get_address(header, "FROM", &addr);
	printf("from: %s\n", addr->printable);
	mu_address_destroy(&addr);
#endif 

#if 0
	mu_header_set_value(header, "Subject", "kakakakaka", 1);
#endif
	mu_header_get_streamref(header, &stream);

	char line[2500];
	size_t n = 0;
#if 0
	rc = mu_stream_readline(stream, line, sizeof(line)-1, off, &n);
	line[n] = 0;
	printf("rc=%d, n=%ld, %s\n", rc, n, line);
#else
	while (mu_stream_readline(stream, line, sizeof(line)-1, &n) == 0
		&& (n != 0))
	{
		if (n >= 1) {
			if (line[n-1] == '\n')
				line[n-1] = 0;
		}
		line[n] = 0;
		printf("%ld: %02x: %s\n", n, line[0], line);
	}
#endif
	mu_stream_destroy(&stream);
	mu_header_destroy(&header);
/*
	mu_header_set_value
	mu_header_aget_value_n
	mu_header_get_address_n
*/
}
