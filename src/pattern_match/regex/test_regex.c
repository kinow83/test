#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <assert.h>
#include "extract.h"


int main(int argc, char **argv)
{
	size_t max_group = 3;
	size_t max_match = 2;
	regex_t compiled;
	regmatch_t groups[max_group];
	char *buf = malloc(SIZE_20MB+1);
	char *regex_string = argv[2];
	char *cursor;

	buf[SIZE_20MB] = 0;

	assert (readfile(argv[1], buf, SIZE_20MB) == SIZE_20MB);

	if (regcomp(&compiled, regex_string, REG_EXTENDED)) {
		printf ("Could not compile regular expression.\n");
		return 1;
	}

	cursor = buf;

	for (int i=0; i<max_match; i++) {
		if (regexec(&compiled, cursor, max_group, groups, 0)) {
			break; // no more match
		}

		int offset = 0;
		for (int k=0; k<max_group; k++) {
			if (groups[k].rm_so == (size_t)-1) {
				break; // no more group
			}
			if (k == 0) {
				offset = groups[k].rm_eo;
			}
#if 0
			char *_copy = strdup(cursor);
			_copy[groups[k].rm_eo] = 0;
			printf("match %u, group %u: [%2u-%2u]: %s\n",
					i, k, groups[k].rm_so, groups[k].rm_eo,
					_copy + groups[k].rm_so);
#endif
			printf("%ld\n", strlen(cursor));
		}
		cursor += offset;
	}
	regfree(&compiled);
	return 0;
}
