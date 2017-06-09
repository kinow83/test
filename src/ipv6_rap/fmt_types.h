
#ifndef FMT_TYPES_H_
#define FMT_TYPES_H_

#define _MAC_FMT_ "%02x:%02x:%02x:%02x:%02x:%02x"

#define _MAC_FMT_FILL_(x) \
	((uint8_t*)(x))[0], ((uint8_t*)(x))[1], ((uint8_t*)(x))[2], ((uint8_t*)(x))[3], ((uint8_t*)(x))[4], ((uint8_t*)(x))[5]

#endif
