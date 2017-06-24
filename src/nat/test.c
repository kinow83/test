/*
 * test.c
 *
 *  Created on: 2017. 6. 19.
 *      Author: root
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifndef __u8
typedef unsigned char __u8;
#endif


struct net_addr_t {
	__u8 size;
	union {
		__u8 ipv4[4];
		__u8 ipv6[16];
	} ip;
};

static inline int net_addr_compare(const struct net_addr_t *a, const struct net_addr_t *b)
{
	if (a->size != b->size) {
		return 1;
	}
	return memcmp((void *)&a->ip, (void *)&b->ip, a->size);
}

static inline unsigned long acl_hash(const struct net_addr_t *a)
{
	unsigned long hash = 0;
	__u8 *ip;
	int i = 0;

	ip = (__u8 *)&a->ip;
	for (; i<sizeof(hash) && i<a->size; i++) {
		hash |= (ip[i] << (i*8));
		printf("hash[%d]> <%08lx>  %04x =shift=> %04x\n",i, hash, ip[i], (ip[i] << i));
	}
	return hash;
}

static void print_net_addr(const struct net_addr_t *a)
{
	int i=0;
	printf("size: %d ", a->size);
	for (;i<a->size; i++) {
		printf("%d.", ((__u8*)&a->ip)[i]);
	}
	printf("\n");
}
int main()
{

	struct net_addr_t ipv4;
	struct net_addr_t ipv6;

	ipv4.size = 4;
	ipv4.ip.ipv4[0] = 10;
	ipv4.ip.ipv4[1] = 10;
	ipv4.ip.ipv4[2] = 200;
	ipv4.ip.ipv4[3] = 150;

	print_net_addr(&ipv4);

	acl_hash(&ipv4);



	ipv6.size = 16;
	for (int i=0; i<ipv6.size; i++) {
		ipv6.ip.ipv6[i] = i+1;
	}
	print_net_addr(&ipv6);

	acl_hash(&ipv6);


	return 0;
}
