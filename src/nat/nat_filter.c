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
	__u8 type;
	union {
		__u8 ipv4[4];
		__u8 ipv6[16];
	};
};

#define VERDICT_ACCEPT
#define VERDICT_DROP;

struct acl_list_t {
	struct list_head list;

	struct net_addr_t net_addr;
	__u8 verdict;
};


