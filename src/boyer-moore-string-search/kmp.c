#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "readfile.h"
#include "bm_timediff.h"

#define MAX_LEN 100
int f[MAX_LEN];

// Compute failure function
void compute_failure_function(char* b, int n) {
	int t, s;
	t = 0;
	f[0] = 0;
	
	for (s = 0; s < n -1; s++) {
		while (t > 0 && b[s+1] != b[t])
			t = f[t-1];
		if (b[s+1] == b[t]) {
			t++;
			f[s+1] = t;
		} else {
			f[s+1] = 0;
		}
	}
}

// Match keyword
int match_keyword(char* a, int m, char* b, int n) {
	int s, i;
	s = 0;
	for (i = 0; i <= m-1; i++) {
		while (s > 0 && a[i] != b[s])
			s = f[s-1];
		if (a[i] == b[s])
			s++;
		if (s == n)
			return i;
	}
	return -1;
}

void info_failure_function(int n) {
	int i;
	printf("----------------------------\n");
	printf("Failure Function            \n");
	printf("----------------------------\n");
	printf("s    |");
	for (i = 0; i < n; i++) 
		printf("%3d", i);
	printf("\n");
	printf("f(s) |");
	for (i = 0; i < n; i++)
		printf("%3d", f[i]);
	printf("\n");
	printf("----------------------------\n");
}

void info_matched_keyword(char* a, int n, int end, char*b ) {
	if (end < 0) {
		printf("NO MATCH\n");
		return;
	}
	printf("%s\n", a);
	int i;
	int begin;
	begin = end -n + 1;
	for (i = 0; i < begin; i++)
		printf(" ");
	for (i = begin; i <= end; i++)
		printf("%c", b[i - begin]);
	printf("\n");
}

void kmp(char* a, int m, char* b, int n) {
	int end;
	compute_failure_function(b, n);
//	info_failure_function(n);
	end = match_keyword(a, m, b, n);
//	info_matched_keyword(a, n, end, b);
	if (end < 0) {
		printf("MISS MATCH\n");
	} else {
		printf("find %d\n", end);
	}
}

int main(int argc, const char *argv[]) {
	char *text = readfile(argv[1]);
	int textlen = strlen(text);
	char *pat = argv[2];
	int patlen = strlen(pat);
	struct timeval tv;
	struct bm_timediff diff_kmp;
	int i;
	
	for (i=0; i<100; i++) {
		init_bm_timediff(&diff_kmp);
		{
			kmp(text, textlen, pat, patlen);
#if 0
			kmp("abababaab", 9, "ababaa", 6);
			kmp("abababbaa", 9, "ababaa", 6);
#endif
		}
		tv = check_bm_timediff("kmp", &diff_kmp, true);
	}

	free(text);
	return 0;
}

/*
[GCC}
gcc kmp.c readfile.c bm_timediff.c -o kmp -O3

kmp: source by github
https://github.com/yaojingguo/c-code/blob/master/algorithm/kmp.c
*/
