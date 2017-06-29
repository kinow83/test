/*
 * rawsock.c
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
#define __FAVOR_BSD           // Use BSD format of tcp header
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

int sock;
unsigned int count = 0;
void sig_handler(int signo)
{
	printf("I Received: %u\n", count);
	close(sock);
	exit(0);
}

void recv_rawsocket()
{
	int ret;
	char pkt[2048];
	struct sockaddr_ll sa = {
			.sll_family = PF_PACKET,
			.sll_halen = ETH_ALEN,
	};

	signal(SIGINT, (void *)sig_handler);

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	assert(sock > 0);

	sa.sll_ifindex = if_nametoindex("enp3s0");
	while (1) {
		ret = recvfrom(sock, pkt, sizeof(pkt), 0, NULL, NULL);
		count++;
		if (ret <= 0) {
			printf("error\n");
			exit(1);
		}
	}
	close(sock);
}

void another_mac_send()
{
	int sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sd == -1) {
		perror("socket");
		exit(1);
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "enp3s0");
	if (ioctl(sd, SIOCGIFINDEX, &ifr) == -1) {
		perror("ioctl");
		exit(1);
	}

	struct sockaddr_ll sll;
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = PF_PACKET;
	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_protocol = htons(ETH_P_IP);
	if (bind(sd, (struct sockaddr*)&sll, sizeof(sll)) == -1) {
		perror("bind");
		exit(1);
	}
	
	uint8_t pkt[1500] = {0, };
	struct ethhdr *eth = (struct ethhdr*)pkt;
	eth->h_source[0] = 0;
	eth->h_source[1] = 1;
	eth->h_source[2] = 2;
	eth->h_source[3] = 3;
	eth->h_source[4] = 4;
	eth->h_source[5] = 5;

	eth->h_dest[0] = 10;
	eth->h_dest[1] = 11;
	eth->h_dest[2] = 12;
	eth->h_dest[3] = 13;
	eth->h_dest[4] = 14;
	eth->h_dest[5] = 15;
	eth->h_proto = htons(ETH_P_IP);

	if (send(sd, pkt, sizeof(pkt), 0) <= 0) {
		perror("send");
	}
}

int
main (int argc, char **argv)
{
#if 0
	int i;
	struct ifreq ifr;
	int sd;
	const char *interface = "enp3s0";
	uint8_t src_mac[6];


	if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
		perror ("socket() failed to get socket descriptor for using ioctl() ");
		exit (EXIT_FAILURE);
	}



	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface);
	ioctl(sd, SIOCGIFHWADDR, &ifr);
	close(sd);

	{
		memcpy(src_mac, ifr.ifr_hwaddr.sa_data, 6*sizeof(uint8_t));
		printf("src mac\n");
		for (i=0; i<5; i++) {
			printf("%02x:", src_mac[i]);
		}
		printf("%02x", src_mac[5]);
	}
#endif

	//another_mac_send();
	recv_rawsocket();
}
