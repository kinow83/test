/*
 * popen_ex.c
 *
 *  Created on: 2016. 9. 8.
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/filter.h>

#define MAXLINE 256

int main() {
	FILE *fp;
	int state;
	char buff[MAXLINE];
	char *tok;
	char *last;

	fp = popen("tcpdump port 22 -ddd", "r");
	if (fp == NULL) {
		perror("erro : ");
		exit(0);
	}


	int filter_cnt = 0;
	struct sock_filter *filters;
	int line = 0;

	if (fgets(buff, MAXLINE, fp)) {
		sscanf(buff, "%d", &filter_cnt);
		printf("filter count: %d\n", filter_cnt);

		if (filter_cnt == 0) {
			return 0;
		}
		filters = (struct sock_filter*)malloc(sizeof(struct sock_filter) * filter_cnt);

		while (fgets(buff, MAXLINE, fp) != NULL && (line < filter_cnt)) {

			printf("%s", buff);

			if ((tok = strtok_r(buff, " ", &last))) {
				sscanf(tok, "%hd", (__u16*)&filters[line].code);
			}
			if ((tok = strtok_r(NULL, " ", &last))) {
				sscanf(tok, "%c", (__u8*)&filters[line].jt);
			}
			if ((tok = strtok_r(NULL, " ", &last))) {
				sscanf(tok, "%c", (__u8*)&filters[line].jf);
			}
			if ((tok = strtok_r(NULL, " ", &last))) {
				sscanf(tok, "%d", (__u32*)&filters[line].k);
			}
			line++;
		}

		for (int i=0; i<filter_cnt; i++) {
			printf("%d %d %d %d\n",
					filters[i].code, filters[i].jt, filters[i].jf, filters[i].k);

		}
	}


	state = pclose(fp);
	printf("state is %d\n", state);
}
