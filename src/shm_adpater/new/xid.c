#include <stdio.h>
#include <string.h>
#include "xid.h"

int gen_xid(xid_t *xid, struct sockaddr_in *addr)
{
	EVP_MD_CTX *md = NULL;
	int mdlen = 0, i;
	uint8_t hashed[32];

	if (!md) {
		md = EVP_MD_CTX_create();
		if (!md) return -1;
	}
	if (EVP_DigestInit_ex(md, EVP_sha256(), NULL) != 1) {
		EVP_MD_CTX_destroy(md);
		md = NULL;
		return -1;
	}
	if (EVP_DigestUpdate(md, addr, sizeof(*addr)) != 1) {
		EVP_MD_CTX_destroy(md);
		md = NULL;
		return -1;
	}
	if (EVP_DigestFinal_ex(md, hashed, &mdlen) != 1) {
		EVP_MD_CTX_destroy(md);
		md = NULL;
		return -1;
	}

	xid->hash = 0xFFFFFFFF;
	for (i=0; i<32; i += 4) {
		// 11111111 11111111 11111111 11111111
		xid->hash ^= hashed[i+0] << 24;
		xid->hash ^= hashed[i+1] << 16;
		xid->hash ^= hashed[i+2] << 8;
		xid->hash ^= hashed[i+3];
	}

	EVP_MD_CTX_destroy(md);
	md = NULL;
	return 0;
}

#if 0
int main()
{
	xid_t x;
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_LOOPBACK;
	gen_xid(&x, &addr);
	printf("%u\n", x.hash);
}
#endif
