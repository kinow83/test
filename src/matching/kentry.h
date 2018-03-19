#ifndef _KENTRY_
#define _KENTRY_

#include <stdint.h>

#define KVALUE_SIZE 256

typedef struct KENTRY {
	uint32_t magic;
	char value[KVALUE_SIZE];
	size_t len;
} KENTRY;

#endif
