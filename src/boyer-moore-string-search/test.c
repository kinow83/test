/*
 * test.c
 *
 *  Created on: 2017. 8. 11.
 *      Author: root
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "readfile.h"
#include "bm_timediff.h"
#include "boyer-moore.h"



int main(int argc, char **argv)
{
	struct timeval tv;
	double time_strstr, time_boyer_moore;
	struct bm_timediff diff_strstr, diff_boyer_moore;
	int pos, ch_compared;
	char *p_pos;
	char *text;
	char *pattern;


	text = readfile(argv[1]);
	pattern = strdup(argv[2]);

	int i=0;
	int win_strstr = 0, win_boyer_moore = 0, draw = 0;
	int textlen = strlen(text);
	int patternlen = strlen(pattern);

	for (i=0; i<5000; i++) {
		usleep(rand() % 10);
		/**********************************************
		 * boyer-moore()
		 **********************************************/
		init_bm_timediff(&diff_boyer_moore);
		{
			pos = 	boyer_moore(text, textlen, (char*)pattern, patternlen, &ch_compared);
		}
		tv = check_bm_timediff("boyer", &diff_boyer_moore, true);
		time_strstr = timeval2double(&tv);

		if (pos == 0 && ch_compared != strlen(pattern)) {
//			printf("missing boyer-moore()\n");
			continue;
		} else {
//			printf("find boyer-moore(): %d\n", pos);
		}

		usleep(rand() % 10);
		/**********************************************
		 * strstr()
		 **********************************************/
		init_bm_timediff(&diff_strstr);
		{
			p_pos = strstr(text, pattern);
		}
		tv = check_bm_timediff("strstr", &diff_strstr, true);
		time_boyer_moore = timeval2double(&tv);

		if (p_pos == NULL) {
//			printf("missing strstr()\n");
			continue;
		} else {
//			printf("find strstr(): %d\n", p_pos-text);
		}

		if (time_strstr > time_boyer_moore) {
			win_strstr++;
		}
		else if (time_strstr < time_boyer_moore) {
			win_boyer_moore++;
		}
		else {
			draw++;
		}
		printf("\n");
	}

	printf("win_boyer_moore: %d, win_strstr: %d, draw: %d\n",
			win_boyer_moore,
			win_strstr,
			draw);

	free(text);
}

/*
[GCC]
gcc test.c bm_timediff.c readfile.c boyer-moore.c -o test -O
gcc test.c bm_timediff.c readfile.c boyer-moore.c -o test -1
gcc test.c bm_timediff.c readfile.c boyer-moore.c -o test -O2
gcc test.c bm_timediff.c readfile.c boyer-moore.c -o test -O3
gcc test.c bm_timediff.c readfile.c boyer-moore.c -o test -Os
*/
