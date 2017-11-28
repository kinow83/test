#include <mailutils/types.h>
#include <mailutils/locus.h>
#include <mailutils/stream.h>

#define STREAM_TYPE mu_stream_t
#define STREAM_PRINTF mu_stream_printf
#define PRINT_LOCUS_POINT mu_stream_print_locus_point
#define PRINT_LOCUS_RANGE mu_stream_print_locus_range
#include "genprloc.c"

