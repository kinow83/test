#ifndef __SAMPLING__
#define __SAMPLING__

#include <stdint.h>

struct word {
	uint32_t magic;
	char text[1024];
	struct word *next;
};

#ifdef __cplusplus 
extern "C" {
#endif
	size_t load_word(int argc, char **argv, size_t minlen, size_t maxlen, struct word **ppw);
	struct word * get_word(struct word *head, size_t n, size_t total);
#ifdef __cplusplus 
}
#endif

#endif
