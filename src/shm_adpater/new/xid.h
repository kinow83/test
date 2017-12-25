#ifndef __XID__
#define __XID__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/evp.h>

typedef struct xid_t {
	uint32_t hash;
} xid_t;

int gen_xid(xid_t *xid, struct sockaddr_in *addr);

#endif
