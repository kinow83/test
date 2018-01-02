#ifndef __DPRINTF__
#define __DPRINTF__

#include <stdio.h>
#include <stdarg.h>

#ifndef NDEBUG
	#define DPRINTF(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#else
	#define DPRINTF
#endif

#endif
