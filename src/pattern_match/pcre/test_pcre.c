#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "extract.h"

// https://www.mitchr.me/SS/exampleCode/AUPG/pcre_example.c.html


int main(int argc, char **argv)
{
	const char *err;
	int offset;
	pcre *compiled;
	pcre_extra *pcre_extra;
	char *buf = malloc(SIZE_20MB+1);
	int sub_vec[30];
	const char *match_str;

	buf[SIZE_20MB] = 0;

	assert (readfile(argv[1], buf, SIZE_20MB) == SIZE_20MB);

	compiled = pcre_compile(argv[2], 0, &err, &offset, NULL);

	if (compiled == NULL) {
		printf("error compile\n");
		exit(1);
	}

	pcre_extra = pcre_study(compiled , 0, &err);
	if (err) {
		printf("error %s\n", err);
		exit(1);
	}

	int ret = pcre_exec(compiled, pcre_extra, buf, SIZE_20MB, 0, 0, sub_vec, 30);
	if (ret < 0) {
		switch (ret) {
			case PCRE_ERROR_NOMATCH      : printf("String did not match the pattern\n");        break;
			case PCRE_ERROR_NULL         : printf("Something was null\n");                      break;
			case PCRE_ERROR_BADOPTION    : printf("A bad option was passed\n");                 break;
			case PCRE_ERROR_BADMAGIC     : printf("Magic number bad (compiled re corrupt?)\n"); break;
			case PCRE_ERROR_UNKNOWN_NODE : printf("Something kooky in the compiled re\n");      break;
			case PCRE_ERROR_NOMEMORY     : printf("Ran out of memory\n");                       break;
			default                      : printf("Unknown error\n");                           break;
		}
	} else {
		printf("match: %d\n", ret);
		if (ret == 0) {
			printf("But too many substrings were found to fit in subStrVec!\n");
			ret = 30 / 3;
		}

		for (int i=0; i<ret; i++) {
			pcre_get_substring(buf, sub_vec, ret, i, &match_str);
			printf("match(%2d/%2d: (%2d,%2d): '%s'\n", i, ret-1, sub_vec[i*2], sub_vec[i*2+1], match_str);
		}
		pcre_free_substring(match_str);
	}


	pcre_free(compiled);
	if (pcre_extra) {
#ifdef PCRE_CONFIG_JIT
		pcre_free_study(pcre_extra);
#else
		pcre_free(pcre_extra);
#endif
	}
	return 0;
}
