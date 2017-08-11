#include <stdio.h>
#include <stdlib.h>
#include "bm_timediff.h"

void init_bm_timediff(struct bm_timediff *td)
{
	gettimeofday(&td->start, NULL);
}

double timeval2double(struct timeval *tv)
{
	char buf[128];
	double d;

	snprintf(buf, sizeof(buf), "%ld.%06ld", tv->tv_sec, tv->tv_usec);
	d = atof(buf);
//	printf("%f = %s\n", d, buf);
	return d;
}

struct timeval check_bm_timediff(const char *desc, struct bm_timediff *td, bool print)
{
	struct timeval e;
	gettimeofday(&td->end, NULL);

	e.tv_sec  = td->end.tv_sec  - td->start.tv_sec;
	e.tv_usec = td->end.tv_usec - td->start.tv_usec;
	if (e.tv_usec < 0) {
		e.tv_sec--;
		e.tv_usec += 1000000;
	}
	if (print) {
		printf("-->[%10s]: %ld.%06ld\n", desc, e.tv_sec, e.tv_usec);
	}
	return e;
}
