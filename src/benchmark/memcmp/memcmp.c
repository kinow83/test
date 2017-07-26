#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "bm_timediff.h"

int memcmp_cmp_test(const char *s1, const char *s2, size_t n)
{
	return memcmp(s1, s2, n);
}

int direct_cmp_test(const char *s1, const char *s2, size_t n)
{
	int i;
	for (i=0; i<n; i++) {
		if (s1[i] != s2[i]) {
			return 1;
		}
	}
	return 0;
}

int memcmp_cmp_mac(unsigned char *m1, unsigned char *m2)
{
	return memcmp(m1, m2, 6);
}

int direct_cmp_mac(unsigned char *m1, unsigned char *m2)
{
	return (m1[0]==m2[0]) && (m1[1]==m2[1]) && (m1[2]==m2[2]) &&
		(m1[3]==m2[3]) && (m1[4]==m2[4]) && (m1[5]==m2[5]);
}

int main()
{
	int i, max = 2000000000;
	struct bm_timediff diff;
	const char *text = "1234567890"; //-=qwertyuiop[];lkjhgfdsazxcvbnm,./";
	size_t textlen = strlen(text);
	unsigned char mac1[6] = {1,2,3,4,5,6};
//	unsigned char mac2[6] = {7,2,3,4,5,6};
	unsigned char mac2[6] = {1,2,3,7,5,6};

	init_bm_timediff(&diff);
	for (i=0; i<max; i++) {
//		direct_cmp_test(text, text, textlen);
		direct_cmp_mac(mac1, mac2);
		time(NULL); // avoid optimization
	}
	check_bm_timediff("direct", &diff);

	init_bm_timediff(&diff);
	for (i=0; i<max; i++) {
//		memcmp_cmp_test(text, text, textlen);
		memcmp_cmp_mac(mac1, mac2);
		time(NULL); // avoid optimization
	}
	check_bm_timediff("memcmp", &diff);

}
