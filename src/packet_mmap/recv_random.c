/*
 * recv_random.c
 *
 *  Created on: 2017. 6. 28.
 *      Author: root
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_TCP, INET_ADDRSTRLEN
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#include <netinet/tcp.h>      // struct tcphdr
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <assert.h>
#include <errno.h>            // errno, perror()
#include <signal.h>
#include <sys/mman.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <poll.h>

/**
https://www.kernel.org/doc/Documentation/networking/packet_mmap.txt

 */

#ifndef likely
# define likely(x)		__builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
# define unlikely(x)		__builtin_expect(!!(x), 0)
#endif

const char *dev = "ens33";

int sd = -1;
unsigned int count = 0;
static int do_exit = 0;

typedef struct gconfig {
	char mode;
} gconfig_t;
gconfig_t g;

void show_sock_buffer(int sock);


void sig_handler(int signo)
{
	printf("catch\n");
	do_exit = 1;
}

void recv_random()
{
	if (sd != -1) close(sd);
	sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sd < 0) {
		perror("socket");
		exit(1);
	}

	show_sock_buffer(sd);

	if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, dev, strlen(dev)) < 0) {
		perror("setsockopt:SO_BINDTODEVICE");
		exit(1);
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, dev);
	if (ioctl(sd, SIOCGIFINDEX, &ifr) == -1) {
		perror("ioctl");
		exit(1);
	}

	struct sockaddr_ll sll;
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = PF_PACKET;
	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_protocol = htons(ETH_P_ALL);
	if (bind(sd, (struct sockaddr*)&sll, sizeof(sll)) == -1) {
		perror("bind");
		exit(1);
	}

	char data[1500];
	while (likely(!do_exit)) {
		if (recvfrom(sd, data, sizeof(data), 0, NULL, NULL) <= 0) {
			perror("recvfrom");
			exit(1);
		}
		count++;
	}
	printf("recv %u\n", count);
	close(sd);
}

void show_sock_buffer(int sock)
{
	int val;
	int valsize = sizeof(val);
	if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &val, &valsize) < 0) {
		perror("getsockopt:SO_RCVBUF");
		exit(1);
	}
	printf("SO_RCVBUF: %d\n", val);

	if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &val, &valsize) < 0) {
		perror("getsockopt:SO_SNDBUF");
		exit(1);
	}
	printf("SO_SNDBUF: %d\n", val);
}

typedef struct block_desc {
	uint32_t version;
	uint32_t offset_to_priv;
	struct tpacket_hdr_v1 h1;
} block_desc_t;

typedef struct ring {
	struct iovec *rd;
	uint8_t *map;
	struct tpacket_req3 req;
} ring_t;

static unsigned long packets_total = 0;
static unsigned long bytes_total = 0;
static unsigned long lose_total = 0;
static unsigned long copy_total = 0;

static void flush_block(block_desc_t *pbd)
{
	pbd->h1.block_status = TP_STATUS_KERNEL;
}
static void walk_block(block_desc_t *pbd, const int block_num)
{
	int num_pkts = pbd->h1.num_pkts;
	int i;
	unsigned long bytes = 0;
	struct tpacket3_hdr *ppd;

	ppd = (struct tpacket3_hdr *)((uint8_t*)pbd +
									pbd->h1.offset_to_first_pkt);
	for (i=0; i<num_pkts; i++) {
		bytes += ppd->tp_snaplen;
		ppd = (struct tpacket3_hdr *)((uint8_t*)ppd +
									ppd->tp_next_offset);
	}
	packets_total += num_pkts;
	bytes_total += bytes;

	if (pbd->h1.block_status & TP_STATUS_COPY) {
		copy_total++;
	}
	if (pbd->h1.block_status & TP_STATUS_LOSING) {
		lose_total++;
	}
}
void recv_tpacket3()
{
	int i;
	int v = TPACKET_V3;

	if (sd != -1) close(sd);
	sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sd < 0) {
		perror("socket");
		exit(1);
	}

	show_sock_buffer(sd);

	if (setsockopt(sd, SOL_PACKET, PACKET_VERSION, &v, sizeof(v)) < 0) {
		perror("setsockopt:PACKET_VERSION");
		exit(1);
	}

	unsigned int blocksiz = 1 << 22;
	unsigned int framesiz = 1 << 11;
	unsigned int blocknum = 64;
	ring_t ring;
	memset(&ring, 0, sizeof(ring));
	ring.req.tp_block_size = blocksiz;
	ring.req.tp_frame_size = framesiz;
	ring.req.tp_block_nr   = blocknum;
	ring.req.tp_frame_nr   = (blocksiz * blocknum) / framesiz;
	ring.req.tp_retire_blk_tov = 60;
	ring.req.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH;

	if (setsockopt(sd, SOL_PACKET, PACKET_RX_RING, &ring.req, sizeof(ring.req)) < 0) {
		perror("setsockopt:PACKET_RX_RING");
		exit(1);
	}
    printf("block tot: %u\n", ring.req.tp_block_size * ring.req.tp_block_nr);
    printf("frame tot: %u\n", ring.req.tp_frame_size * ring.req.tp_frame_nr);


	ring.map = mmap(NULL, ring.req.tp_block_size * ring.req.tp_block_nr,
			PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_LOCKED,
			sd,
			0);
	if (ring.map == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	ring.rd = malloc(ring.req.tp_block_nr * sizeof(*ring.rd));
	assert(ring.rd);
	for (i=0; i<ring.req.tp_block_nr; i++) {
		ring.rd[i].iov_base = ring.map + (i*ring.req.tp_block_size);
		ring.rd[i].iov_len = ring.req.tp_block_size;
	}

	/*
	 * Binding the socket to your network interface is mandatory (with zero copy) to
	 * know the header size of frames used in the circular buffer.
	 */
	struct sockaddr_ll sll;
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = PF_PACKET;
	sll.sll_protocol = htons(ETH_P_ALL);
	sll.sll_ifindex = if_nametoindex(dev);
	sll.sll_hatype = 0;
	sll.sll_pkttype = 0;
	sll.sll_halen = 0;

	if (bind(sd, (struct sockaddr*)&sll, sizeof(sll)) < 0) {
		perror("bind");
		exit(1);
	}

	unsigned int block_num = 0;
	unsigned int blocks    = 64;
	block_desc_t *pbd;
	struct pollfd pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.fd = sd;
	pfd.events = POLLIN | POLLERR;
	pfd.revents = 0;

	while (likely(!do_exit)) {
		pbd = (block_desc_t *)ring.rd[block_num].iov_base;
		if ((pbd->h1.block_status & TP_STATUS_USER) == 0) {
			int ret = poll(&pfd, 1, -1);
			if (ret < 0) {
				if (errno != EINTR) {
					perror("poll");
				}
			}
			continue;
		}
		walk_block(pbd, block_num);
		flush_block(pbd);
		block_num = (block_num + 1) % blocks;
	}

	struct tpacket_stats_v3 stats;
	int statssize = sizeof(stats);
	if (getsockopt(sd, SOL_PACKET, PACKET_STATISTICS, &stats, &statssize) < 0) {
		perror("getsockopt:PACKET_STATISTICS");
		exit(1);
	}
	fflush(stdout);
	printf("\n");
	printf("recv %d packets, %u bytes, %u dropped, freeze_q_cnt: %u, lose: %u, copy: %u\n",
			stats.tp_packets, bytes_total, stats.tp_drops, stats.tp_freeze_q_cnt, lose_total, copy_total);

	munmap(ring.map, ring.req.tp_block_size * ring.req.tp_block_nr);
	free(ring.rd);
	close(sd);

}



int
main (int argc, char **argv)
{
	int opt;


	memset(&g, 0, sizeof(g));
	while ((opt = getopt(argc, argv, "ku")) != -1) {
		switch (opt) {
		case 'k':
			g.mode = 'k';
			break;
		case 'u':
			g.mode = 'u';
			break;
		}
	}

	signal(SIGINT, (void *)sig_handler);

	if (g.mode == 'k') {
		printf("using packet kernel\n");
		recv_random();
	} else {
		printf("using packet mmap\n");
		recv_tpacket3();
	}
}
