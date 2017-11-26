#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "mailutils/mailutils.h"
#include "bm_timediff.h"

pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER;

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

void get_subject(char *buf, int print) {
	int rc;
	char *pval = NULL;
	char *plang = NULL;
	char *sb = NULL;
	char *sb2 = NULL;
	mu_header_t h;

	rc = mu_header_create (&h, buf, strlen(buf), NULL);
	rc = mu_header_aget_value(h, "Subject", &sb);
	sb2 = no_crlf(sb);
	pthread_mutex_lock(&lock); {
		rc = mu_mimehdr_decode_param (sb2, 0, "UTF-8", &pval, &plang);
	} pthread_mutex_unlock(&lock);
	if (print && pval) printf("Subject ==> %s\n", pval);

	if (pval) free(pval);
	if (plang) free(plang);
	if (sb) free(sb);
	if (sb2) free(sb2);
	mu_header_destroy (&h, NULL);
}

void get_subject_n(char *buf, int n, int print) {
	int rc;
	char *pval = NULL;
	char *plang = NULL;
	char *sb = NULL;
	char *sb2 = NULL;
	mu_header_t h;
	char subject[100];

	snprintf(subject, sizeof(subject), "Subject%d", n);

	rc = mu_header_create (&h, buf, strlen(buf), NULL);
	rc = mu_header_aget_value(h, subject, &sb);
	sb2 = no_crlf(sb);
	pthread_mutex_lock(&lock); {
		rc = mu_mimehdr_decode_param (sb2, 0, "UTF-8", &pval, &plang);
	} pthread_mutex_unlock(&lock);
	if (print && pval) printf("Subject%d ==> %s\n", n, pval);

	if (pval) free(pval);
	if (plang) free(plang);
	if (sb) free(sb);
	if (sb2) free(sb2);
	mu_header_destroy (&h, NULL);
}

void get_content_type(char *buf, int print) {
	int rc;
	char *pval = NULL;
	char *plang = NULL;
	char *content = NULL;
	char *content2 = NULL;
	mu_header_t h;

	rc = mu_header_create (&h, buf, strlen(buf), NULL);
	rc = mu_header_aget_value (h, "Content-Type", &content);
	content2 = no_crlf(content);

#if 1
	int flags;
	char *name = NULL;
	rc = mu_mimehdr_aget_param (content2, "name", &name, &flags);
	pthread_mutex_lock(&lock); {
		rc = mu_mimehdr_decode_param (name, flags, "UTF-8", &pval, &plang);
	} pthread_mutex_unlock(&lock);
	free(name);
#else
	rc = mu_mimehdr_aget_decoded_param (content2, "name", "UTF-8", &pval, &plang);
#endif
	if (print && pval) printf("Content-Type ==> %s\n", pval);

	if (pval) free(pval);
	if (plang) free(plang);
	if (content) free(content);
	if (content2) free(content2);
	mu_header_destroy (&h, NULL);
}

void get_content_disp(char *buf, int print) {
	int rc;
	char *pval = NULL;
	char *plang = NULL;
	char *content = NULL;
	mu_header_t h;

	rc = mu_header_create (&h, buf, strlen(buf), NULL);
	rc = mu_header_aget_value (h, "Content-Disposition", &content);
#if 1
	int flags;
	char *name = NULL;
	rc = mu_mimehdr_aget_param (content, "filename", &name, &flags);
	pthread_mutex_lock(&lock); {
		rc = mu_mimehdr_decode_param (name, flags, "UTF-8", &pval, &plang);
	} pthread_mutex_unlock(&lock);
	free(name);
#else
	rc = mu_mimehdr_aget_decoded_param (content, "filename", "UTF-8", &pval, &plang);
#endif
	if (print && pval) printf("Content-Disp ==> %s\n", pval);
	if (pval) free(pval);
	if (plang) free(plang);
	if (content) free(content);
	pval = NULL;
	plang = NULL;

	mu_header_destroy (&h, NULL);
}

void get_content_disp_n(char *buf, int n, int print) {
	int rc;
	char *pval = NULL;
	char *plang = NULL;
	char *content = NULL;
	mu_header_t h;
	char disp[100];

	snprintf(disp, sizeof(disp), "Content-Disposition%d", n);

	rc = mu_header_create (&h, buf, strlen(buf), NULL);
	rc = mu_header_aget_value (h, disp, &content);
#if 1
	int flags;
	char *name = NULL;
	rc = mu_mimehdr_aget_param (content, "filename", &name, &flags);
	pthread_mutex_lock(&lock); {
		rc = mu_mimehdr_decode_param (name, flags, "UTF-8", &pval, &plang);
	} pthread_mutex_unlock(&lock);
	free(name);
#else
	rc = mu_mimehdr_aget_decoded_param (content, "filename", "UTF-8", &pval, &plang);
#endif
	if (print && pval) printf("%s ==> %s\n", disp, pval);
	if (pval) free(pval);
	if (plang) free(plang);
	if (content) free(content);
	pval = NULL;
	plang = NULL;

	mu_header_destroy (&h, NULL);
}

void get_content_type_n(char *buf, int print) {
	int rc;
	char *pval = NULL;
	char *plang = NULL;
	char *content = NULL;
	char *value = NULL;
	mu_header_t h;

	rc = mu_header_create (&h, buf, strlen(buf), NULL);
	rc = mu_header_aget_value (h, "Content-Type1", &content);
	rc = mu_mimehdr_aget_decoded_param (content, "URL", "UTF-8", &pval, &plang);
	if (print && pval) printf("Content-Type1 ==> %s\n", pval);
	if (pval) free(pval);
	if (plang) free(plang);
	if (content) free(content);
	pval = NULL;
	plang = NULL;

	rc = mu_header_aget_value (h, "Content-Type2", &content);
	rc = mu_mimehdr_aget_decoded_param (content, "title", "UTF-8", &pval, &plang);
	if (print && pval) printf("Content-Type2 ==> %s\n", pval);
	if (pval) free(pval);
	if (plang) free(plang);
	if (content) free(content);
	pval = NULL;
	plang = NULL;

	rc = mu_header_aget_value (h, "Content-Type3", &content);
	rc = mu_mimehdr_aget_decoded_param (content, "title", "UTF-8", &pval, &plang);
	if (print && pval) printf("Content-Type3 ==> %s\n", pval);
	if (pval) free(pval);
	if (plang) free(plang);
	if (content) free(content);
	pval = NULL;
	plang = NULL;

	mu_header_destroy (&h, NULL);
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

	struct bm_timediff td;
	int i=0;
	pthread_t *ts = malloc(sizeof(pthread_t)*num_threads);
	init_bm_timediff(&td);
	{
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
