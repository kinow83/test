#ifndef _BENCHMARK_TIMEDIFF_
#define _BENCHMARK_TIMEDIFF_

#include <stdbool.h>
#include <sys/time.h>

struct bm_timediff
{
	struct timeval start;
	struct timeval end;
};

void init_bm_timediff(struct bm_timediff *td);
double timeval2double(struct timeval *tv);
struct timeval check_bm_timediff(const char *desc, struct bm_timediff *td, bool print);

#endif
