#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <net/if.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/if_ether.h>
#include <linux/rtnetlink.h>
#include <netpacket/packet.h>

#include "fmt_types.h"

struct ether_addr{
	uint8_t a[ETH_ALEN];
} __attribute__ ((__packed__));

struct net_env {
	char iface[IFNAMSIZ];
	int ifindex;
};


void show_all_local_addr()
{
	int i = 0;
	struct ifaddrs *ifs, *ptr;
	struct sockaddr_ll *sockpptr;
	struct sockaddr_in6 *sockin6ptr;
	struct ether_addr ether;
	char ipv6str[128];

	if (getifaddrs(&ifs)) {
		return -1;
	}

	for (ptr=ifs; ptr; ptr=ptr->ifa_next) {
		if (ptr->ifa_addr == NULL) {
			continue;
		}
		if (ptr->ifa_name == NULL) {
			continue;
		}
		printf("[%d]->%s %d\n", ++i, ptr->ifa_name, (ptr->ifa_addr)->sa_family);

		if ((ptr->ifa_addr)->sa_family == AF_PACKET) {
			sockpptr = (struct sockaddr_ll *)(ptr->ifa_addr);
			printf("\tEthernet: ");
			if (sockpptr->sll_halen == ETH_ALEN) {
				memcpy(&ether, sockpptr->sll_addr, ETH_ALEN);
				printf("mac: "_MAC_FMT_"\n", _MAC_FMT_FILL_(&ether));
			}
		}
		else if ((ptr->ifa_addr)->sa_family == AF_INET6) {
			sockin6ptr = (struct sockaddr_in6 *)(ptr->ifa_addr);
				printf("\tIPv6: ");
			if (IN6_IS_ADDR_LINKLOCAL( &(sockin6ptr->sin6_addr))){
				printf("link local address: ");
			} else {
				;
			}

			if (inet_ntop(AF_INET6, &(sockin6ptr->sin6_addr), ipv6str, sizeof(ipv6str))) {
				printf("%s\n", ipv6str);
			}
		}
		else if ((ptr->ifa_addr)->sa_family == AF_INET) {
			printf("\tIPv4: \n");
		}
	}

	freeifaddrs(ifs);
}

int get_local_addr(const char *iface)
{
	int i = 0;
	struct ifaddrs *ifs, *ptr;
	struct sockaddr_ll *sockpptr;
	struct sockaddr_in6 *sockin6ptr;
	struct ether_addr ether;
	char ipv6str[128];

	if (getifaddrs(&ifs)) {
		return -1;
	}

	for (ptr=ifs; ptr; ptr=ptr->ifa_next) {
		if (ptr->ifa_addr == NULL) {
			continue;
		}
		if (ptr->ifa_name == NULL) {
			continue;
		}
		printf("[%d]->%s %d\n", ++i, ptr->ifa_name, (ptr->ifa_addr)->sa_family);

		if ((ptr->ifa_addr)->sa_family == AF_PACKET) {
			sockpptr = (struct sockaddr_ll *)(ptr->ifa_addr);
			printf("\tEthernet: ");
			if (sockpptr->sll_halen == ETH_ALEN) {
				memcpy(&ether, sockpptr->sll_addr, ETH_ALEN);
				printf("mac: "_MAC_FMT_"\n", _MAC_FMT_FILL_(&ether));
			}
		}
		else if ((ptr->ifa_addr)->sa_family == AF_INET6) {
			sockin6ptr = (struct sockaddr_in6 *)(ptr->ifa_addr);
				printf("\tIPv6: ");
			if (IN6_IS_ADDR_LINKLOCAL( &(sockin6ptr->sin6_addr))){
				printf("link local address: ");
			} else {
				;
			}

			if (inet_ntop(AF_INET6, &(sockin6ptr->sin6_addr), ipv6str, sizeof(ipv6str))) {
				printf("%s\n", ipv6str);
			}
		}
		else if ((ptr->ifa_addr)->sa_family == AF_INET) {
			printf("\tIPv4: \n");
		}
	}

	freeifaddrs(ifs);
}


int main(int argc, char **argv)
{
	struct net_env env;

	strcpy(env.iface, "eth0");
	env.ifindex = if_nametoindex(env.iface);

	printf("ifindex: %d\n", env.ifindex);
	printf("AF_PACKET: %d\n", AF_PACKET);
	printf("AF_INET6: %d\n", AF_INET6);

	get_local_addr(env.iface);
}
