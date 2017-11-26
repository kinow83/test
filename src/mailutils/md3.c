#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#include "mailutils/mailutils.h"
#include "bm_timediff.h"

char *no_crlf(char *buf) {
	int len = strlen(buf);
	char *p = malloc(len+1);
	int i;
	int pos;
	for (i=0, pos=0; i<len; i++) {
		if (buf[i] == '\r' || buf[i] == '\n' || buf[i] == '\t') {
			continue;
		}
		p[pos++] = buf[i];
	}
	p[pos] = 0;
	return p;
}


static int print_subject (const char *name, void *item, void *data)
{
	struct mu_mime_param *param = item;
	printf ("Subject ==> %s\n", param->value);
	return 0;
}

void get_subject(char *buf, int print) {
	int rc;
	char *sb = NULL;
	char *sb2 = NULL;
	mu_header_t h;
	struct mu_mime_param *param;

	rc = mu_header_create (&h, buf, strlen(buf));
	rc = mu_header_aget_value(h, "Subject", &sb);
	sb2 = no_crlf(sb);
	rc = mu_rfc2047_decode_param ("UTF-8", sb2, &param);
	if (print && param->value) printf("Subject ==> %s\n", param->value);
	if (sb) free(sb);
	if (sb2) free(sb2);
	
    mu_mime_param_free (param);
	mu_header_destroy (&h);
}

void get_subject_n(char *buf, int n, int print) {
	int rc;
	char *sb = NULL;
	char *sb2 = NULL;
	mu_header_t h;
	char subject[100];
	struct mu_mime_param *param;

	snprintf(subject, sizeof(subject), "Subject%d", n);

	rc = mu_header_create (&h, buf, strlen(buf));
	rc = mu_header_aget_value(h, subject, &sb);
	sb2 = no_crlf(sb);
	rc = mu_rfc2047_decode_param ("UTF-8", sb2, &param);
	if (print && param->value) printf("Subject%d ==> %s\n", n, param->value);

	if (sb) free(sb);
	if (sb2) free(sb2);

    mu_mime_param_free (param);
	mu_header_destroy (&h);
}

void get_content_type_n(char *buf, int print) {
	int rc;
	char *pval = NULL;
	char *plang = NULL;
	char *content = NULL;
	mu_header_t h;

	rc = mu_header_create (&h, buf, strlen(buf));
	rc = mu_header_aget_value (h, "Content-Type1", &content);
	rc = mu_mimehdr_aget_decoded_param (content, "URL", "UTF-8", &pval, &plang);
	if (print) printf("Content-Type1 ==> %s\n", pval);
	if (content) free(content);
	if (pval) free(pval);
	if (plang) free(plang);
	pval = NULL;
	plang = NULL;

	rc = mu_header_aget_value (h, "Content-Type2", &content);
	rc = mu_mimehdr_aget_decoded_param (content, "title", "UTF-8", &pval, &plang);
	if (print) printf("Content-Type2 ==> %s\n", pval);
	if (content) free(content);
	if (pval) free(pval);
	if (plang) free(plang);
	pval = NULL;
	plang = NULL;

	rc = mu_header_aget_value (h, "Content-Type3", &content);
	rc = mu_mimehdr_aget_decoded_param (content, "title", "UTF-8", &pval, &plang);
	if (print) printf("Content-Type3 ==> %s\n", pval);
	if (content) free(content);
	if (pval) free(pval);
	if (plang) free(plang);
	pval = NULL;
	plang = NULL;
	
	mu_header_destroy (&h);
}

void get_content_type(char *buf, int print) {
	int rc;
	char *pval = NULL;
	char *plang = NULL;
	char *content = NULL;
	char *content2 = NULL;
	char *value = NULL;
	int flags;
	mu_header_t h;

	rc = mu_header_create (&h, buf, strlen(buf));
	rc = mu_header_aget_value (h, "Content-Type", &content);
	content2 = no_crlf(content);
	rc = mu_mimehdr_aget_decoded_param (content2, "name", "UTF-8", &pval, &plang);
	if (print) printf("Content-Type ==> %s\n", pval);
	if (content) free(content);
	if (content2) free(content2);
	if (pval) free(pval);
	if (plang) free(plang);
	pval = NULL;
	plang = NULL;

	mu_header_destroy (&h);
}

void get_content_disp(char *buf, int print) {
	int rc;
	char *pval = NULL;
	char *plang = NULL;
	char *content = NULL;
	char *value = NULL;
	int flags;
	mu_header_t h;

	rc = mu_header_create (&h, buf, strlen(buf));
	rc = mu_header_aget_value (h, "Content-Disposition", &content);
	rc = mu_mimehdr_aget_decoded_param (content, "filename", "UTF-8", &pval, &plang);
	if (print) printf("Content-Disp ==> %s\n", pval);
	if (content) free(content);
	if (pval) free(pval);
	if (plang) free(plang);
	pval = NULL;
	plang = NULL;

	mu_header_destroy (&h);
}

void get_content_disp_n(char *buf, int n, int print) {
	int rc;
	char *pval = NULL;
	char *plang = NULL;
	char *content = NULL;
	char *value = NULL;
	int flags;
	mu_header_t h;
	char disp[100];

	snprintf(disp, sizeof(disp), "Content-Disposition%d", n);

	rc = mu_header_create (&h, buf, strlen(buf));
	rc = mu_header_aget_value (h, disp, &content);
	rc = mu_mimehdr_aget_decoded_param (content, "filename", "UTF-8", &pval, &plang);
	if (print) printf("%s ==> %s\n", disp, pval);
	if (content) free(content);
	if (pval) free(pval);
	if (plang) free(plang);
	pval = NULL;
	plang = NULL;

	mu_header_destroy (&h);
}


int num_threads = 8;
int num_loop = 100;
void *threadfunc(void *data) {
	char *buf = (char *)data;
	int i;

	for (i=0; i<num_loop; i++) {
		get_subject(buf, 0);
		get_content_type(buf, 0);
		get_content_disp(buf, 0);
	}
}
int main(int argc, char **argv)
{
	int rc;
	char *mbox = argv[1];
	struct stat st;
	char *buf;
	FILE *fp;
	mu_stream_t stream;
	num_loop = atoi(argv[2]);

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

#if 0
	get_subject(buf, 1);
	get_subject_n(buf, 1, 1);
	get_subject_n(buf, 2, 1);
	get_subject_n(buf, 3, 1);
	get_subject_n(buf, 4, 1);
	get_subject_n(buf, 5, 1);
	get_subject_n(buf, 6, 1);
	get_subject_n(buf, 7, 1);
	get_subject_n(buf, 8, 1);
	get_subject_n(buf, 9, 1);
	get_subject_n(buf, 10, 1);
	get_subject_n(buf, 11, 1);
	get_content_type(buf, 1);
	get_content_type_n(buf, 1);
	get_content_disp(buf, 1);
	get_content_disp_n(buf, 1, 1);
#endif

	int i=0;
	struct bm_timediff td;
	init_bm_timediff(&td);
	{
		pthread_t *ts = malloc(sizeof(pthread_t)*num_threads);
		for (i=0; i<num_threads; i++) {
			pthread_create(&ts[i], NULL, threadfunc, buf);
		}
		for (i=0; i<num_threads; i++) {
			pthread_join(ts[i], NULL);
		}
	}
	check_bm_timediff("md2", &td, 1);

	free(buf);
	return 0;
}
