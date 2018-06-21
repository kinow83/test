#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include <stdint.h>
#include "bm_timediff.h"

typedef struct bmctx {
	uint8_t data[DATA_SIZE];
} bmctx_t;

typedef struct bmconfig {
	uint32_t id;
	uint32_t count;
	struct bm_timediff td;
}bmconfig_t;

#endif
