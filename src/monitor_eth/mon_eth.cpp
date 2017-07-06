#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/types.h>
#include <linux/filter.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "mon_eth.h"
#include "hash.h"

bool running = false;

struct arphdr
{
	uint16_t htype;       /* hardware type (must be ARPHRD_ETHER) */
	uint16_t ptype;       /* protocol type (must be ETH_P_IP) */
	uint8_t  hlen;        /* hardware address length (must be 6) */
	uint8_t  plen;        /* protocol address length (must be 4) */
	uint16_t op;          /* ARP opcode */
	uint8_t  s_hwaddr[6]; /* sender's hardware address */
	uint8_t  s_ipaddr[4]; /* sender's IP address */
	uint8_t  t_hwaddr[6]; /* target's hardware address */
	uint8_t  t_ipaddr[4]; /* target's IP address */
	uint8_t  pad[18];     /* pad for min. Ethernet payload (60 bytes) */
};

void node_print(const char*desc, node_t* node)
{
	printf("%s ", desc);
	printf("ipaddr:"_IP_FMT_" ", _IP_FMT_FILL_32_(node->ipaddr));
	printf("hwaddr:"_MAC_FMT_"\n", _MAC_FMT_FILL_(node->hwaddr));
}

void notify_node_print(const char*desc, notify_node_t* node)
{
	printf("%s ", desc);
	printf("ipaddr:"_IP_FMT_" ", _IP_FMT_FILL_32_(node->old_node.ipaddr));
	printf("hwaddr:"_MAC_FMT_" => ", _MAC_FMT_FILL_(node->old_node.hwaddr));
	printf("hwaddr:"_MAC_FMT_"\n", _MAC_FMT_FILL_(node->new_node.hwaddr));
}

static bool same_subnet(uint32_t network, uint32_t netmask, uint32_t check_ipaddr)
{
	return (network == (check_ipaddr & netmask));
}

static int node_compare(const void* a, const void* b)
{
	node_t *pa = (node_t*)a;
	node_t *pb = (node_t*)b;
	if (pa->ipaddr != pb->ipaddr) return 1;
//	if (memcmp(pa->hwaddr, pa->hwaddr, sizeof(pa->hwaddr))) return 1;
	return 0;
}

static uint16_t node_hash(const void* d)
{
	node_t *pd = (node_t*)d;
	return (uint16_t)pd->ipaddr;
}

static bool get_eth_info(const char *dev, uint8_t *hwaddr, uint32_t *ipaddr, uint32_t *netmask)
{
	int sock;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		fprintf(stderr, "socket: %s\n", strerror(errno));
		return false;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name)-1);
	if (ipaddr) {
		if (ioctl(sock, SIOCGIFADDR, &ifr) == 0) {
			*ipaddr = ((struct sockaddr_in *)(&(ifr.ifr_addr)))->sin_addr.s_addr;
		} else {
			fprintf(stderr, "ioctl: SIOCGIFADDR: %s\n", strerror(errno));
			*ipaddr = 0;
		}
	}
	if (netmask) {
		if (ioctl(sock, SIOCGIFNETMASK, &ifr) == 0) {
			*netmask = ((struct sockaddr_in *)(&(ifr.ifr_addr)))->sin_addr.s_addr;
		} else {
			fprintf(stderr,"ioctl: SIOCGIFNETMASK: %s\n", strerror(errno));
			*netmask = 0;
		}
	}
	if (hwaddr) {
		if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
			memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, 6);
		} else {
			fprintf(stderr, "ioctl: SIOCGIFHWADDR: %s\n", strerror(errno));
		}
	}
	close (sock);
	return true;
}

static int create_socket(const char *dev, int proto)
{
	int sock = -1;
	int on = 1;

	sock = socket(PF_PACKET, SOCK_RAW, htons(proto));
	if (sock == -1)
	{
		fprintf(stderr, "Failed to create socket - %s\n", strerror(errno));
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) == -1)
	{
		fprintf(stderr, "Failed to setsockopt SO_BROADCAST - %s\n", 
			strerror(errno));
		close(sock);
		return -1;
	}

	/* Set the device to use */
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name)-1);

	/* Get the current flags that the device might have */
	if (ioctl (sock, SIOCGIFFLAGS, &ifr) == -1) {
		fprintf(stderr, "ioctl(SIOCGIFFLAGS) failed - %s\n", strerror(errno));
		close(sock);
		return -1;
	}
    /* Set the old flags plus the IFF_PROMISC flag */
    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl (sock, SIOCSIFFLAGS, &ifr) == -1) {
		fprintf(stderr, "ioctl(SIOCSIFFLAGS) failed - %s\n", strerror(errno));
		close(sock);
		return -1;
    }
    /* Get the Socket Index */
    if(ioctl(sock, SIOCGIFINDEX, &ifr) == -1) {
        fprintf(stderr, "ioctl(SIOCGIFINDEX) failed - %s\n", strerror(errno));
        close(sock);
        return -1;
    }
    /* Bind Socket */
    struct sockaddr_ll  sll;
    bzero(&sll, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(proto);
    if(bind(sock, (struct sockaddr*)&sll, sizeof(sll)) == -1) {
        fprintf(stderr, "bind failed - %s\n", strerror(errno));
        close(sock);
        return -1;
    }

    return sock;
}

static void eth_print(struct ethhdr *ethhdr)
{
	printf("0x%04x ", ntohs(ethhdr->h_proto));
	printf("hw src:"_MAC_FMT_" ", _MAC_FMT_FILL_(ethhdr->h_source));
	printf("hw dst:"_MAC_FMT_" ", _MAC_FMT_FILL_(ethhdr->h_dest));
}

static void ip_print(struct ethhdr *ethhdr, struct iphdr *iphdr)
{
	eth_print(ethhdr);
	printf("ip src:"_IP_FMT_" ", _IP_FMT_FILL_32_(iphdr->saddr));
	printf("ip dst:"_IP_FMT_"\n", _IP_FMT_FILL_32_(iphdr->daddr));
}

static void arp_print(struct ethhdr *ethhdr, struct arphdr *arphdr)
{
	eth_print(ethhdr);
	uint16_t op = ntohs(arphdr->op);
	if (op == 1/*ARPOP_REQUEST*/) {
		printf("op:%s ", "REQ");
	} else if (op ==  2/*ARPOP_REPLY*/) {
		printf("op:%s ", "REP");
	}
	printf("s_hwaddr:"_MAC_FMT_" ", _MAC_FMT_FILL_(arphdr->s_hwaddr));
	printf("s_ipaddr:"_IP_FMT_" ", _IP_FMT_FILL_(arphdr->s_ipaddr));
	printf("t_hwaddr:"_MAC_FMT_" ", _MAC_FMT_FILL_(arphdr->t_hwaddr));
	printf("t_ipaddr:"_IP_FMT_"\n", _IP_FMT_FILL_(arphdr->t_ipaddr));
}

void monitoring(const char *dev, 
	void* (*notify)(const notify_node_t*))
{
	uint8_t hwaddr[6];
	uint32_t ipaddr;
	uint32_t netmask;
	uint32_t network;
	int sock;
	fd_set fdset;
	struct ethhdr *ethhdr = NULL;
	struct iphdr *iphdr = NULL;
	struct arphdr *arphdr = NULL;
	uint16_t arp_op;
	// max size ip and arp packet
	static int data_size = sizeof(struct ethhdr) + 
		(sizeof(struct iphdr) > sizeof(struct arphdr) ? 
			sizeof(struct iphdr) : sizeof(struct arphdr));
	uint8_t data[data_size];
	struct timeval tm;
	int bytes;
	uint16_t hproto;
	hash_table_t *hash_table = NULL;
	node_t node;
	node_t *pnode = NULL;
	node_t *find_node = NULL;
	notify_node_t notify_node;

	if (!get_eth_info(dev, hwaddr, &ipaddr, &netmask)) {
		fprintf(stderr, "Failed to get '%s' information\n", dev);
		return;
	}
	network = ipaddr & netmask;

	sock = create_socket(dev, ETH_P_ALL);
	if (sock == -1) {
		fprintf(stderr, "Failed to create '%s' sock\n", dev);
		return;
	}

	hash_table = hash_table_create(node_compare, node_hash, free, true, 65532);
	if (!hash_table) {
		fprintf(stderr, "Failed to create table table\n");
		close(sock);
		return;
	}

	running = true;

	while (running) {
		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);

		// wait time
		tm.tv_sec  = 0;
		tm.tv_usec = 10000;

		if (select(sock + 1, &fdset, (fd_set *)NULL, (fd_set *)NULL, &tm) < 0) {
			continue;
		}
		else if (FD_ISSET(sock, &fdset)) {
			bytes = recv(sock, &data, data_size, 0);
			if (bytes <= 0) {
				continue;
			}
			ethhdr = (struct ethhdr *)&data;
			hproto = ntohs(ethhdr->h_proto);
			if (hproto == ETH_P_ARP) {
				arphdr = (struct arphdr *)&data[sizeof(struct ethhdr)];
				arp_op = ntohs(arphdr->op);
				if (arp_op == 1) {
					memcpy(&node.ipaddr, arphdr->s_ipaddr, sizeof(node.ipaddr));
					memcpy(node.hwaddr, arphdr->s_hwaddr, sizeof(node.hwaddr));
				} else if (arp_op == 2) {
					memcpy(&node.ipaddr, arphdr->t_ipaddr, sizeof(node.ipaddr));
					memcpy(node.hwaddr, arphdr->t_hwaddr, sizeof(node.hwaddr));
				} else {
					continue;
				}
//				arp_print(ethhdr, arphdr);
			} else if (hproto == ETH_P_IP) {
				iphdr = (struct iphdr *)&data[sizeof(struct ethhdr)];
				node.ipaddr = iphdr->saddr;
				memcpy(node.hwaddr, ethhdr->h_source, sizeof(node.hwaddr));
//				ip_print(ethhdr, iphdr);
			} else {
				continue;
			}
			if (!same_subnet(network, netmask, node.ipaddr)) {
				continue;
			}

			find_node = (node_t*)hash_table_finddata(hash_table, &node);
			if (find_node == NULL) { // insert node
				pnode = (node_t*)malloc(sizeof(node));
				memcpy(pnode, &node, sizeof(node));
				if (!hash_table_insert(hash_table, pnode)) {
					free(pnode);
					fprintf(stderr, "failed to insert hash table\n");
					exit(1);
				}
				node_print(" NEW ===>", &node);
			} else { // comapre hwaddr
				if (memcmp(find_node->hwaddr, node.hwaddr, sizeof(node.hwaddr))) {
					memcpy(&notify_node.old_node, find_node, sizeof(node_t));
					memcpy(&notify_node.new_node, &node, sizeof(node_t));
					notify(&notify_node);

					// update new node
					pnode = (node_t*)malloc(sizeof(node));
					memcpy(pnode, &node, sizeof(node));
					if (!hash_table_insert(hash_table, pnode)) {
						free(pnode);
						fprintf(stderr, "failed to insert hash table\n");
						exit(1);
					}
				}
			}
		}
	}
	hash_table_free(hash_table);
	close(sock);
	return;
}

