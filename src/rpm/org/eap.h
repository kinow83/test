#ifndef __LIB_EAP__
#define __LIB_EAP__

typedef struct libeap_t {
	void *ctx;
} libeap_t;


libeap_t *new_eap();

#endif
