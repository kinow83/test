#ifndef __LIB_TLS__
#define __LIB_TLS__

typedef struct libtls_t {
	void *ctx;
} libtls_t;


libtls_t *new_tls();

#endif
