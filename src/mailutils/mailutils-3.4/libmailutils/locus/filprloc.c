#include <stdio.h>

#define STREAM_TYPE FILE*
#define STREAM_PRINTF fprintf
#define PRINT_LOCUS_POINT mu_file_print_locus_point
#define PRINT_LOCUS_RANGE mu_file_print_locus_range
#include "genprloc.c"
