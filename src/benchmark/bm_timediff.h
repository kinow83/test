#include <sys/time.h>

#ifndef _BENCHMARK_TIMEDIFF_
#define _BENCHMARK_TIMEDIFF_

struct bm_timediff
{
	struct timeval start;
	struct timeval end;
};

void init_bm_timediff(struct bm_timediff *td);
struct timeval check_bm_timediff(const char *desc, struct bm_timediff *td);

#endif
