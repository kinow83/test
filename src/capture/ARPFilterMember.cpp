/*
 * ARPFilterMember.cpp
 *
 *  Created on: 2016. 9. 26.
 *      Author: root
 */

#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <stdint.h>
#include <linux/filter.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "WlanUtils.h"
#include "ARPFilterMember.h"

namespace kinow {

void ARPFilterMember::filterHandler(uint8_t* buf, size_t len) {
	struct ethhdr *ether = NULL;
	struct arp_message *p_arp = NULL;
	struct dot1q_arp_message *p1q_arp = NULL;
	struct arpinfo arpinfo;
	uint8_t *pad = NULL;
	uint32_t magic = 0;

	ether = (struct ethhdr *)buf;

	if (_IS_ZERO_MAC_(ether->h_source) || (ether->h_source[0] & 0x01)) {
		return;
	}

	memset(&arpinfo, 0, sizeof(arpinfo));

	// 802.1Q (vlan)
	if (ether->h_proto == htons(ETH_P_8021Q)) {

	}
	// 802.3 - Ethernet (ARP)
	else if (ether->h_proto == htons(ETH_P_ARP)) {
		p_arp = (struct arp_message *)buf;

		// ARP_REPLY / ARP_REQUEST
		if (p_arp->operation == htons(ARPOP_REPLY) ||
			p_arp->operation == htons(ARPOP_REQUEST))
		{
			_MAC_COPY_(arpinfo.mac, p_arp->sHaddr);
			arpinfo.ip = *((uint32_t *)p_arp->sInaddr);

			// ARPOP_REPLY
			if (p_arp->operation == htons(ARPOP_REPLY)) {
				if (ARPFilterMember::validAnnounceARP(
						p_arp->sInaddr, p_arp->sHaddr,
						p_arp->tInaddr, p_arp->tHaddr,
						(struct announce_data *)p_arp->pad))
				{
					ARPFilterMember::showAnnounceData(
							"recv",
							*((uint32_t*)p_arp->tInaddr),
							(struct announce_data *)p_arp->pad);
				}
			}

			if (arpinfo.ip == 0 || arpinfo.ip == 0xFFFFFFFF) {
				return;
			}
			if (_IS_ZERO_MAC_(arpinfo.mac) || _IS_BCAST_MAC_(arpinfo.mac)) {
				return;
			}
			arpinfo.timestamp = time(NULL);
			showArpinfo(&arpinfo);
		}
		else {
			return;
		}
	}
}

bool ARPFilterMember::validAnnounceARP(uint8_t *sInaddr, uint8_t *sHaddr, uint8_t *tInaddr, uint8_t *tHaddr, struct announce_data *anno) {
	// For performance. check magic key
	if (anno->magic != htonl(ANNOUNCE_ARP_PAD_MAGIC)) {
		return false;
	}
	if (_IS_ZERO_MAC_(sHaddr) && _IS_ZERO_MAC_(tHaddr)
			&& _IS_ZERO_IP_(sInaddr) && !_IS_ZERO_IP_(tInaddr)) {
		return true;
	}
	return false;
}

void ARPFilterMember::showAnnounceData(const char* title, uint32_t sender_ipaddr, struct announce_data *anno) {
	fprintf(stdout, "%s [announce] "_IP_FMT_" -> ip:"_IP_FMT_", \tmac:"_MAC_FMT_"\n",
			title,
			_IP_FMT_FILL_(sender_ipaddr),
			_IP_FMT_FILL_(anno->announce_ipaddr),
			_MAC_FMT_FILL_(anno->announce_macaddr));
}

void ARPFilterMember::showArpinfo(struct arpinfo *arp) {
	fprintf(stdout, "time: %lu, ip:"_IP_FMT_", \tmac:"_MAC_FMT_"\n",
			arp->timestamp, _IP_FMT_FILL_(arp->ip), _MAC_FMT_FILL_(arp->mac));
}


bool ARPFilterMember::sendAnnounceARP(
		const char *ifname,
		uint32_t announce_ipaddr, uint8_t *announce_macaddr,
		uint32_t sender_ipaddr, uint8_t *sender_macaddr)
{
	uint8_t arp_msg[sizeof(struct arp_message)]; // ARP buffer
	struct arp_message *p_arp = NULL;
	int sock = -1;
	bool ok = false;
	struct announce_data *announce = NULL; // padding of ARP packets

	sock = WlanUtils::createSocket(SOCK_RAW, ETH_P_ALL);
	if (sock == -1) {
		fprintf(stderr, "ARPFilterMember: sendAnnounceARP Failed to create socket - %s\n", strerror(errno));
		goto END_SEND;
	}
	if (WlanUtils::setSockOptBroadcat(sock) == false) {
		fprintf(stderr, "ARPFilterMember: sendAnnounceARP Failed to setsockopt SO_BROADCAST - %s\n", strerror(errno));
		goto END_SEND;
	}
	if (WlanUtils::setSockOptBindDevice(sock, ifname) == false) {
		fprintf(stderr, "ARPFilterMember: sendAnnounceARP Failed to bind device - %s\n", strerror(errno));
		goto END_SEND;
	}
	if (!WlanUtils::setSockBind(sock, ETH_P_ALL, ifname)) {
		fprintf(stderr, "ARPFilterMember: sendAnnounceARP Failed to bind(): %s\n", strerror(errno));
		goto END_SEND;
	}

	p_arp = (struct arp_message *) &arp_msg;
	memset(p_arp, 0, sizeof(arp_msg));
	_MAC_COPY_(p_arp->ethhdr.h_dest, MAC_BCAST_ADDR); /* MAC DA */
	_MAC_COPY_(p_arp->ethhdr.h_source, sender_macaddr);/* MAC SA */
	p_arp->ethhdr.h_proto = htons(ETH_P_ARP);         /* protocol type (Ethernet) */
	p_arp->htype = htons(ARPHRD_ETHER);               /* hardware type */
	p_arp->ptype = htons(ETH_P_IP);                   /* protocol type (ARP message) */
	p_arp->hlen = 6;                                  /* hardware address length */
	p_arp->plen = 4;                                  /* protocol address length */
	p_arp->operation = htons(ARPOP_REPLY);            /* ARP op code */
//	*((uint32_t *) p_arp->sInaddr) = 0;               /* source IP address */
	*((uint32_t *) p_arp->tInaddr) = sender_ipaddr;   /* target IP address */
//	_MAC_COPY_(p_arp->sHaddr, 0);                     /* source hardware address */
//	_MAC_COPY_(p_arp->tHaddr, 0);                     /* target hardware address */


	// build announce ARP datas
	announce = (struct announce_data *)p_arp->pad;
	announce->magic = htonl(ANNOUNCE_ARP_PAD_MAGIC);
	_MAC_COPY_(announce->announce_macaddr, announce_macaddr);
	announce->announce_ipaddr = announce_ipaddr;

	// shoot ARP
	if (send(sock, &arp_msg, sizeof(arp_msg), 0) <= 0) {
		ARPFilterMember::showAnnounceData("failure", sender_ipaddr, announce);
	} else {
		ARPFilterMember::showAnnounceData("success", sender_ipaddr, announce);
	}

END_SEND:
	if (sock != -1)
		close(sock);
	return ok;
}


bool ARPFilterMember::sendARPRequest(
		const char *ifname,
		uint32_t sender_ipaddr, uint8_t *sender_macaddr,
		uint32_t target_ipaddr, uint8_t *target_macaddr)
{
	uint8_t arp_msg[sizeof(struct arp_message)]; // ARP buffer
	struct arp_message *p_arp = NULL;
	int sock = -1;
	bool ok = false;

	sock = WlanUtils::createSocket(SOCK_RAW, ETH_P_ALL);
	if (sock == -1) {
		fprintf(stderr, "ARPFilterMember: sendARPRequest Failed to create socket - %s\n", strerror(errno));
		goto END_SEND;
	}
	if (WlanUtils::setSockOptBroadcat(sock) == false) {
		fprintf(stderr, "ARPFilterMember: sendARPRequest Failed to setsockopt SO_BROADCAST - %s\n", strerror(errno));
		goto END_SEND;
	}
	if (WlanUtils::setSockOptBindDevice(sock, ifname) == false) {
		fprintf(stderr, "ARPFilterMember: sendARPRequest Failed to bind device - %s\n", strerror(errno));
		goto END_SEND;
	}
	if (!WlanUtils::setSockBind(sock, ETH_P_ALL, ifname)) {
		fprintf(stderr, "ARPFilterMember: sendARPRequest Failed to bind(): %s\n", strerror(errno));
		goto END_SEND;
	}

	p_arp = (struct arp_message *) &arp_msg;
	memset(p_arp, 0, sizeof(arp_msg));
	_MAC_COPY_(p_arp->ethhdr.h_dest, MAC_BCAST_ADDR); /* MAC DA */
	_MAC_COPY_(p_arp->ethhdr.h_source, sender_macaddr);/* MAC SA */
	p_arp->ethhdr.h_proto = htons(ETH_P_ARP);         /* protocol type (Ethernet) */
	p_arp->htype = htons(ARPHRD_ETHER);               /* hardware type */
	p_arp->ptype = htons(ETH_P_IP);                   /* protocol type (ARP message) */
	p_arp->hlen = 6;                                  /* hardware address length */
	p_arp->plen = 4;                                  /* protocol address length */
	p_arp->operation = htons(ARPOP_REQUEST);          /* ARP op code */
	*((uint32_t *) p_arp->sInaddr) = sender_ipaddr;   /* source IP address */
	*((uint32_t *) p_arp->tInaddr) = target_ipaddr;   /* target IP address */
	_MAC_COPY_(p_arp->sHaddr, sender_macaddr);        /* source hardware address */
	_MAC_COPY_(p_arp->tHaddr, target_macaddr);        /* target hardware address */

	// shoot ARP
	if (send(sock, &arp_msg, sizeof(arp_msg), 0) <= 0) {

	} else {

	}

END_SEND:
	if (sock != -1)
		close(sock);
	return ok;
}

void showARPPacket(const char *title, struct arp_message *arp) {
	fprintf(stdout, "%s [ARP] [%s] \n", title,
			(arp->operation == htons(ARPOP_REQUEST)) ? "request" : "reply  ");
	fprintf(stdout, "sender "_IP_FMT_", "_MAC_FMT_"\n",
			_IP_FMT_FILL_(arp->sInaddr), _MAC_FMT_FILL_(arp->sHaddr));
	fprintf(stdout, "target "_IP_FMT_", "_MAC_FMT_"\n",
			_IP_FMT_FILL_(arp->tInaddr), _MAC_FMT_FILL_(arp->tHaddr));
}

} /* namespace kinow */
