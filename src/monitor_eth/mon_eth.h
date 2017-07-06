#include <stdint.h>

#ifndef __MON_ETH__
#define __MON_ETH__

#ifndef _IP_FMT_
	#define _IP_FMT_ "%d.%d.%d.%d"
#endif

#ifndef _IP_FMT_FILL_
	#if __BYTE_ORDER == __LITTLE_ENDIAN
		#define _IP_FMT_FILL_(x) \
			((uint8_t*)(x))[0], \
			((uint8_t*)(x))[1], \
			((uint8_t*)(x))[2], \
			((uint8_t*)(x))[3]
	#else
		#define _IP_FMT_FILL_(x) \
			((uint8_t*)(x))[3], \
			((uint8_t*)(x))[2], \
			((uint8_t*)(x))[1], \
			((uint8_t*)(x))[0]
	#endif
#endif

#ifndef _IP_FMT_FILL_32_
	#if __BYTE_ORDER == __LITTLE_ENDIAN
		#define _IP_FMT_FILL_32_(x) \
			((uint8_t*)(&x))[0], \
			((uint8_t*)(&x))[1], \
			((uint8_t*)(&x))[2], \
			((uint8_t*)(&x))[3]
	#else
		#define _IP_FMT_FILL_32_(x) \
			((uint8_t*)(&x))[3], \
			((uint8_t*)(&x))[2], \
			((uint8_t*)(&x))[1], \
			((uint8_t*)(&x))[0]
	#endif
#endif

#ifndef _MAC_FMT_
	#define _MAC_FMT_ "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#ifndef _MAC_FMT_FILL_
	#define _MAC_FMT_FILL_(x) \
		((uint8_t*)(x))[0], \
		((uint8_t*)(x))[1], \
		((uint8_t*)(x))[2], \
		((uint8_t*)(x))[3], \
		((uint8_t*)(x))[4], \
		((uint8_t*)(x))[5]
#endif

#ifndef _MAC_FMT_FILL_64_
	#define _MAC_FMT_FILL_64_(x) \
		((uint8_t*)(&x))[5], \
		((uint8_t*)(&x))[4], \
		((uint8_t*)(&x))[3], \
		((uint8_t*)(&x))[2], \
		((uint8_t*)(&x))[1], \
		((uint8_t*)(&x))[0]
#endif

typedef struct node_t {
	uint32_t ipaddr;
	uint8_t hwaddr[6];
} node_t;

typedef struct notify_node_t {
	node_t old_node;
	node_t new_node;
} notify_node_t;


void monitoring(const char *dev, void* (*notify)(const notify_node_t*));
void node_print(const char*desc, node_t* node);
void notify_node_print(const char*desc, notify_node_t* node);

#endif
