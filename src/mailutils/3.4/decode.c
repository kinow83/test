#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mailutils/header.h"
#include "mailutils/stream.h"
#include "mailutils/address.h"
#include "mailutils/mime.h"
#include "mailutils/message.h"
#include "mailutils/assoc.h"

char *no_cr(const char *p) {
	size_t n = strlen(p);
	char *_p = malloc(n+1);
	size_t i, pos;

	for (i=0, pos=0; i<n; i++) {
		if (p[i] == '\r' || p[i] =='\n' || p[i] == '\t') continue;
		_p[pos++] = p[i];
	}
	_p[pos] = 0;
	return _p;
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

//	printf("%s\n", buf);

	// last: \n
	rc = mu_header_create(&header, buf, st.st_size+1);
	free(buf);
	if (rc) {
		exit(1);
	}

	char *subject = NULL, *ptr = NULL;

#if 1
	rc = mu_header_aget_value(header, "Subject", &ptr);
	printf("1.subject: %s\n", ptr);
	ptr = no_cr(ptr);
	printf("ptr:%s\n", ptr);
	rc = mu_rfc2047_decode("UTF-8", ptr, &subject);
	printf("2.subject:\n%s\n", subject);
#endif
	printf("\n\n\n\n\n");

	char *disp = NULL;
	rc = mu_header_aget_value(header, "Content-Disposition", &ptr);
	ptr = no_cr(ptr);
	printf("ptr:%s\n", ptr);
#if 1 // one pass
	rc = mu_mimehdr_aget_decoded_param(ptr, "filename", "UTF-8", &disp, NULL);
#endif
	printf("disp:\n%s\n", disp);





//	rc = mu_header_aget_value(header, "Content-Diposition", &);
}
