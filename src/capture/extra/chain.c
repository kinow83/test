/*
 * chain.c
 *
 *  Created on: 2016. 9. 8.
 *      Author: root
 */
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/filter.h>
#include <errno.h>


#define ETHER_TYPE	0x0800

#define DEFAULT_IF	"eth0"
#define BUF_SIZ		1024

struct named_sock {
	char name[128];
	int sock;
};

struct sock_array {
	int count;
	struct named_sock** socks;
};

struct socket_filter_st {
	u_int count;
	struct sock_filter* filters;
};


void clearSockArray(struct sock_array *arr) {
	int i;
	if (arr) {
		if (arr->socks) {
			for (i=0; i<arr->count; i++) {
				free(arr->socks[i]);
			}
		}
		free(arr->socks);
	}
}

void clearSocketFilters(struct socket_filter_st* sf) {
	if (sf) {
		if (sf->count > 0 && sf->filters) {
			free(sf->filters);
		}
	}
}

struct socket_filter_st* buildSocketFilters(const char* expression) {
	FILE *fp;
	char buf[256];
	char tcpdumpExpression[256];
	char *tok;
	char *last;
	int filter_cnt = 0;
	struct sock_filter *filters;
	int line = 0;
	struct socket_filter_st* sf;

	sf = (struct socket_filter_st*)malloc(sizeof(struct socket_filter_st));
	memset(sf, 0, sizeof(struct socket_filter_st));

	snprintf(tcpdumpExpression, sizeof(tcpdumpExpression),
			"/usr/sbin/tcpdump %s -ddd", expression);

	fp = popen(tcpdumpExpression, "r");
	if (fp == NULL) {
		free(sf);
		return NULL;
	}

	if (fgets(buf, sizeof(buf), fp)) {
		// BPF filter count
		sscanf(buf, "%d", &filter_cnt);

		if (filter_cnt == 0) {
			free(sf);
			pclose(fp);
			return NULL;
		}

		filters = (struct sock_filter*)malloc(sizeof(struct sock_filter) * filter_cnt);

		while (fgets(buf, sizeof(buf), fp) != NULL && (line < filter_cnt)) {
			// BPF Filter block code
			// Actual filter code
			if ((tok = strtok_r(buf, " ", &last))) {
				sscanf(tok, "%hd", (__u16*)&filters[line].code);
			}
			// Jump true
			if ((tok = strtok_r(NULL, " ", &last))) {
				sscanf(tok, "%u", (__u32*)&filters[line].jt);
			}
			// Jump false
			if ((tok = strtok_r(NULL, " ", &last))) {
				sscanf(tok, "%u", (__u32*)&filters[line].jf);
			}
			// Generic multiuse field
			if ((tok = strtok_r(NULL, " ", &last))) {
				sscanf(tok, "%d", (__u32*)&filters[line].k);
			}
			line++;
		}
		sf->count = filter_cnt;
		sf->filters = filters;
		pclose(fp);
		return sf;
	}

	free(sf);
	pclose(fp);
	return NULL;
}

void debug_sock_filter(struct socket_filter_st* sf) {
	int i = 0;
	int count = 0;

	if (sf) {
		count = sf->count;
		fprintf(stdout, "%d\n", count);
		for (i=0; i<count; i++) {
			fprintf(stdout, "{ 0x%x, %d, %d, 0x%08x },\n",
					sf->filters[i].code, sf->filters[i].jt,
					sf->filters[i].jf, sf->filters[i].k);
		}
	}
}

#define WITH_FILTER

int make_sock(const char *ifname, const char* exp) {
	int s;
	struct ifreq ifopts;
	int sockopt = 1;
	struct socket_filter_st* sf = NULL;
	const char* expression = NULL;

	printf("call make_sock (%s, %s)\n", ifname, exp);

#ifdef WITH_FILTER
	expression = exp;
#endif

	if ((s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		perror("listener: socket");
		return -1;
	}

	if (expression != NULL && expression[0]) {
		sf = buildSocketFilters(expression);
		printf("[make_sock] sock_filter count: %d\n", sf->count);
	} else {
		printf("skip socket filter\n");
	}

	strncpy(ifopts.ifr_name, ifname, IFNAMSIZ-1);
	ioctl(s, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(s, SIOCSIFFLAGS, &ifopts);

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("SO_REUSEADDR");
		close(s);
		if (sf) clearSocketFilters(sf);
		return -1;
	}

	printf("[make_sock] ifname: %s\n", ifname);
	if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, ifname, IFNAMSIZ-1) == -1)	{
		perror("SO_BINDTODEVICE");
		close(s);
		if (sf) clearSocketFilters(sf);
		return -1;
	}

	if (expression != NULL && expression[0]) {
		struct sock_fprog bpf;
		bpf.len = sf->count;
		bpf.filter = sf->filters;
		if(setsockopt(s, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf)) < 0) {
			perror("SO_ATTACH_FILTER");
			close(s);
			if (sf) clearSocketFilters(sf);
			return -1;
		}
	}

#if 0
    if(ioctl(s, SIOCGIFINDEX, &ifopts) == -1) {
		perror("SIOCGIFINDEX");
		close(s);
		if (sf) clearSocketFilters(sf);
		return -1;
    }


    struct sockaddr_ll  sll;
    bzero(&sll, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifopts.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);
    if(bind(s, (struct sockaddr*)&sll, sizeof(sll)) == -1) {
		perror("bind");
		close(s);
		clearSocketFilters(sf);
		return -1;
    }
#endif

	printf("[make_sock] success: %s -> %d\n", ifname, s);
	if (sf) debug_sock_filter(sf);
	return s;
}

struct sock_array* build_sock_array(const char* ifname) {

	struct sock_array* arr = malloc(sizeof(struct sock_array));
	struct named_sock *s1, *s2, *s3;

	arr->socks = malloc(sizeof(struct named_sock*) * 3);

	arr->count = 3;

	printf("[build_sock_array] do port 22\n");
	s1 = malloc(sizeof(struct named_sock));
	strcpy(s1->name, "port 22");
	s1->sock = make_sock(ifname, "port 22");
	arr->socks[0] = s1;
	printf("[build_sock_array] done port 22\n");

	printf("[build_sock_array] do port 80\n");
	s2 = malloc(sizeof(struct named_sock));
	strcpy(s2->name, "port 80");
	s2->sock = make_sock(ifname, "port 80");
	arr->socks[1] = s2;
	printf("[build_sock_array] done port 80\n");

	printf("[build_sock_array] do port 53\n");
	s3 = malloc(sizeof(struct named_sock));
	strcpy(s3->name, "port 53");
	s3->sock = make_sock(ifname, "port 53");
	arr->socks[2] = s3;
	printf("[build_sock_array] done port 53\n");

	return arr;
}

void debugHex(const char* title, uint8_t *buf, int len) {
	int i;
	printf("%s (%d)\n", title, len);
	for (i=0; i<len; i++) {
		printf("%02x ", buf[i]);
	}
	printf("\n");
}

void chainFilterRecv(struct sock_array *arr) {
	int i;
	struct named_sock *s;
	uint8_t buf[BUF_SIZ];
	int numbytes;
	struct ip *iph;
	struct tcphdr *tcph;
	struct ether_header *ep;
	unsigned short ether_type;
	u_char* packet;

//	printf("call chainFilterRecv\n");

	for (i=0; i<arr->count; i++) {
		s = arr->socks[i];
repeat:
		numbytes = recv(s->sock, buf, BUF_SIZ, MSG_DONTWAIT /* MSG_PEEK | MSG_DONTWAIT */);
//		printf("%s(%d) : %d\n", s->name, s->sock, numbytes);
		if (numbytes < 0) {
			if (errno == EINTR) {
				goto repeat;
			}
//			perror("chainFilterRecv::recv()");
			return;
		}
		else if (numbytes == 0) {
			goto repeat;
		}
		else if (numbytes > 0) {
			packet = buf;
			ep = (struct ether_header *) packet;
			packet += sizeof(struct ether_header);
			ether_type = ntohs(ep->ether_type);

//			printf("   0x%04x   %p\n", ether_type, packet);
//			debugHex(s->name, buf, numbytes);

			if (ether_type == ETHERTYPE_IP) {
				// IP 헤더에서 데이타 정보를 출력한다.
				iph = (struct ip *) packet;
				printf("[%d/%d][%d:%s]\t", i+1, arr->count, s->sock, s->name);
				printf("src: %s -> ", inet_ntoa(iph->ip_src));
				printf("dst: %s", inet_ntoa(iph->ip_dst));

				if (iph->ip_p == IPPROTO_TCP) {
					tcph = (struct tcphdr *) (packet + iph->ip_hl * 4);
					printf("[%d -> ", ntohs(tcph->source));
					printf("%d]", ntohs(tcph->dest));
				}
				printf("\n");
			}
		}
	}
}



int main(int argc, char *argv[])
{
	char sender[INET6_ADDRSTRLEN];
	char targer[INET6_ADDRSTRLEN];
	int sockfd, ret, i;
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	struct ifreq if_ip;	/* get ip addr */
	struct sockaddr_storage saddr;
	struct sockaddr_storage daddr;
	uint8_t buf[BUF_SIZ];
	u_char* packet;
	char ifName[IFNAMSIZ];

	/* Get interface name */
	if (argc > 1)
		strcpy(ifName, argv[1]);
	else
		strcpy(ifName, DEFAULT_IF);

	memset(&if_ip, 0, sizeof(struct ifreq));

	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		perror("listener: socket");
		return -1;
	}

	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);

	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("SO_REUSEADDR");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		perror("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

#if 0
	// tcpdump ip and tcp and port 80 -dd
	struct sock_filter code[]= {
			{ 0x28, 0, 0, 0x0000000c },
			{ 0x15, 0, 10, 0x00000800 },
			{ 0x30, 0, 0, 0x00000017 },
			{ 0x15, 0, 8, 0x00000006 },
			{ 0x28, 0, 0, 0x00000014 },
			{ 0x45, 6, 0, 0x00001fff },
			{ 0xb1, 0, 0, 0x0000000e },
			{ 0x48, 0, 0, 0x0000000e },
			{ 0x15, 2, 0, 0x00000050 },
			{ 0x48, 0, 0, 0x00000010 },
			{ 0x15, 0, 1, 0x00000050 },
			{ 0x6, 0, 0, 0x00040000 },
			{ 0x6, 0, 0, 0x00000000 },
	};
	struct sock_fprog bpf;
	bpf.len = sizeof(code)/sizeof(struct sock_filter); /* TDB */
	bpf.filter = code;
	if(setsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf)) < 0) {
		perror("SO_ATTACH_FILTER");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
#endif


	struct sock_array* arr = build_sock_array(ifName);
	if (!arr) {
		printf("failed to build_sock_array\n");
		exit(0);
	}

	getchar();


	while (1) {

		chainFilterRecv(arr);

		numbytes = recv(sockfd, buf, BUF_SIZ, 0);
//		printf("listener: got packet %lu bytes\n", numbytes);

		if (numbytes > 0) {
			struct ip *iph;
			struct tcphdr *tcph;
			struct ether_header *ep;
			unsigned short ether_type;

//			debugHex("recv", buf, numbytes);

			packet = buf;
			ep = (struct ether_header *) packet;
			packet += sizeof(struct ether_header);
			ether_type = ntohs(ep->ether_type);

			if (ether_type == ETHERTYPE_IP) {
				// IP 헤더에서 데이타 정보를 출력한다.
				iph = (struct ip *) packet;
				printf("     src: %s -> ", inet_ntoa(iph->ip_src));
				printf("dst: %s", inet_ntoa(iph->ip_dst));

				if (iph->ip_p == IPPROTO_TCP) {
					tcph = (struct tcphdr *) (packet + iph->ip_hl * 4);
					printf("[%d -> ", ntohs(tcph->source));
					printf("%d]", ntohs(tcph->dest));
				}
				printf("\n\n");
			}
		}
	}
	close(sockfd);
	return 0;
}
