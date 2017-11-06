#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "bm_timediff.h"


char strs[10][100];

const char *pstr[10] = {
	"1111111111111111111111111111111111111111111111111111111",
	"2222222222222222222222222222222222222222222222222222222",
	"3333333333333333333333333333333333333333333333333333333",
	"4444444444444444444444444444444444444444444444444444444",
	"5555555555555555555555555555555555555555555555555555555",
	"6666666666666666666666666666666666666666666666666666666",
	"7777777777777777777777777777777777777777777777777777777",
	"8888888888888888888888888888888888888888888888888888888",
	"9999999999999999999999999999999999999999999999999999999",
	"0000000000000000000000000000000000000000000000000000000",
};
int count;

void test_snprintf() {
	count=0;
	for (int i=0; i<10; i++) {
		count++;
		snprintf(strs[i], 100, "%s", pstr[i]);
	}
}

void test_memcpy() {
	count=0;
	size_t len;
	for (int i=0; i<10; i++) {
		len=strlen(pstr[i]);
		len=(len>=99) ? 98 : len;
		memcpy(strs[i], pstr[i], len);
		strs[i][len+1] = 0;
	}
}

void test_strncpy() {
	count=0;
	size_t len;
	for (int i=0; i<10; i++) {
		strncpy(strs[i], pstr[i], 99);
		strs[i][99] = 0;
	}
}

int main()
{
	int count = 10000000;
	int i;
	struct bm_timediff diff;

	init_bm_timediff(&diff);
	for (i=0; i<count; i++)
		test_memcpy();
	check_bm_timediff("memcpy", &diff, 1);

	init_bm_timediff(&diff);
	for (i=0; i<count; i++)
		test_snprintf();
	check_bm_timediff("snprintf", &diff, 1);

	init_bm_timediff(&diff);
	for (i=0; i<count; i++)
		test_strncpy();
	check_bm_timediff("strncpy", &diff, 1);

}

/*
kaka snprintf # ./str_ben 
-->[    memcpy]: 1.308361
-->[  snprintf]: 12.651125
-->[   strncpy]: 1.339835
kaka snprintf # ./str_ben 
-->[    memcpy]: 1.288250
-->[  snprintf]: 12.565658
-->[   strncpy]: 1.323991
*/
