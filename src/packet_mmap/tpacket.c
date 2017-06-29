/*
 * tpacket.c
 *
 *  Created on: 2017. 6. 29.
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
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <assert.h>
#include <errno.h>            // errno, perror()


void set_tpacket_version(int ver)
{
	int sd;
	sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	assert(sd > 0);

	socklen_t valsize = sizeof(ver);
	if (setsockopt(sd, SOL_PACKET, PACKET_VERSION, (void*)&ver, valsize) < 0) {
		perror("getsockopt");
		exit(1);
	}
	printf("success = (%d)\n", ver);

	close(sd);
}

void show_tpacket_version()
{
	const char *p;
	int sd;
	sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	assert(sd > 0);

	int val = -1;
	socklen_t valsize = sizeof(val);
	if (getsockopt(sd, SOL_PACKET, PACKET_VERSION, &val, &valsize) < 0) {
		perror("getsockopt");
		exit(1);
	}
	switch (val) {
	case TPACKET_V1:
		p = "TPACKET_V1";
		break;
	case TPACKET_V2:
		p = "TPACKET_V2";
		break;
	case TPACKET_V3:
		p = "TPACKET_V3";
		break;
	default:
		p = "???";
	}

	printf("tpacket ver = %s (%d)\n", p, val);

	close(sd);
}

int main()
{
	show_tpacket_version();
	set_tpacket_version(TPACKET_V1);
	set_tpacket_version(TPACKET_V2);
	set_tpacket_version(TPACKET_V3);
}
