/*
 * nat_filter.c
 *
 *  Created on: 2017. 6. 13.
 *      Author: root
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <error.h>
#include <stdio.h>
#include <linux/list.h>

struct net_addr_t {
	__u8 size;
	union {
		__u8 ipv4[4];
		__u8 ipv6[16];
	} ip;
};

#define VERDICT_ACCEPT
#define VERDICT_DROP;

struct acl_t {
	struct list_head list;

	struct net_addr_t net_addr;
	__u8 verdict;
};

static inline int net_addr_compare(const struct net_addr_t *a, const struct net_addr_t *b)
{
	if (a->size != b->size) {
		return 1;
	}
	return memcmp((void *)&a->ip, (void *)&b->ip, a->size);
}

static inline int acl_compare(const struct acl_t *a, const struct acl_t *b)
{
	return net_addr_compare(&a->net_addr, &b->net_addr);
}

static inline unsigned long acl_hash(const struct acl_t *a)
{
	unsigned long hash = 0;
	__u8 *ip;
	int i = 0;

	ip = (__u8 *)&a->net_addr.ip;
	for (; i<sizeof(hash) && i<a->net_addr.size; i++) {
		hash |= (ip[i] << (i*8));
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

