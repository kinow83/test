/*
 * send_random.c
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


int sd;
unsigned int count = 0;
void sig_handler(int signo)
{
	printf("I send: %u\n", count);
	close(sd);
	exit(0);
}

int main()
{
	signal(SIGALRM, sig_handler);

	sd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sd < 0) {
		perror("socket");
		exit(1);
	}

	const char *dev = "vmnet8";
	if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, dev, strlen(dev)) < 0) {
		perror("setsockopt:SO_BINDTODEVICE");
		exit(1);
	}

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(4000);
	saddr.sin_addr.s_addr = inet_addr("192.168.98.132");

	char data[1500];

	alarm(5);
	while (1) {
		if (sendto(sd, data, sizeof(data), 0, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
			perror("sendto");
		}
		count++;
	}

	close(sd);

}

