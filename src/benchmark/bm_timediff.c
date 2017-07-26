#include <stdio.h>
#include "bm_timediff.h"

void init_bm_timediff(struct bm_timediff *td)
{
	gettimeofday(&td->start, NULL);
}

struct timeval check_bm_timediff(const char *desc, struct bm_timediff *td)
{
	struct timeval e;
	gettimeofday(&td->end, NULL);

	e.tv_sec  = td->end.tv_sec  - td->start.tv_sec;
	e.tv_usec = td->end.tv_usec - td->end.tv_usec;
	if (e.tv_usec < 0) {
		e.tv_sec--;
		e.tv_usec += 1000000;
	}
	printf("--[%s]-------------------------------\n", desc);
	printf("elapsed: %ld.%ld\n", e.tv_sec, e.tv_usec);
	return e;
}
