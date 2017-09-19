/*
 * ARPFilterMember.h
 *
 *  Created on: 2016. 9. 26.
 *      Author: root
 */

#ifndef CAPTURE_ARPFILTERMEMBER_H_
#define CAPTURE_ARPFILTERMEMBER_H_


#include <sys/prctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/types.h>
#include <stdint.h>
#include <linux/if_arp.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>    /* struct sockaddr_ll */
#include <linux/wireless.h>     /* srtuct iwreq */
#include <errno.h>
#include <sys/types.h>
#include <linux/filter.h>
#include <fcntl.h>
#include <time.h>

#include "IFilterMember.h"
#include "Error.h"

namespace kinow {

#define ANNOUNCE_ARP_PAD_MAGIC 0x19830421

/**
 * struct announce_data
 * - announce IP and MAC which learned in network.
 */
struct announce_data {
	uint32_t magic;
	uint8_t  announce_macaddr[8];
	uint32_t announce_ipaddr;
}__attribute__((packed)) ;

//////////////////////////////////////////////
// 802.1Q header
//////////////////////////////////////////////
/**
 * struct vlan_ethhdr - vlan ethernet header (ethhdr + vlan_hdr)
 * @h_dest: destination ethernet address
 * @h_source: source ethernet address
 * @h_vlan_proto: ethernet protocol (always 0x8100)
 * @h_vlan_tag: priority and VLAN ID
 * @h_vlan_encapsulated_proto: packet type ID or len
 */
struct vlan_ethhdr {
    uint8_t h_dest[ETH_ALEN];
    uint8_t h_source[ETH_ALEN];
    uint16_t h_vlan_proto;
    uint16_t h_vlan_tag;
    uint16_t h_proto;
}__attribute__((packed)) ;

/**
 * struct arp_message - ARP packet format
 */
struct arp_message
{
    struct ethhdr ethhdr; /* Ethernet header */
    uint16_t htype; /* hardware type (must be ARPHRD_ETHER) */
    uint16_t ptype; /* protocol type (must be ETH_P_IP) */
    uint8_t hlen; /* hardware address length (must be 6) */
    uint8_t plen; /* protocol address length (must be 4) */
    uint16_t operation; /* ARP opcode */
    uint8_t sHaddr[6]; /* sender's hardware address */
    uint8_t sInaddr[4]; /* sender's IP address */
    uint8_t tHaddr[6]; /* target's hardware address */
    uint8_t tInaddr[4]; /* target's IP address */
    uint8_t pad[18]; /* pad for min. Ethernet payload (60 bytes) */
} __attribute__((packed)) ;

/**
 * struct dot1q_arp_message - ARP packet format with 802.1Q
 */
struct dot1q_arp_message
{
    struct vlan_ethhdr ethhdr; /* Ethernet VLAN header */
    uint16_t htype; /* hardware type (must be ARPHRD_ETHER) */
    uint16_t ptype; /* protocol type (must be ETH_P_IP) */
    uint8_t hlen; /* hardware address length (must be 6) */
    uint8_t plen; /* protocol address length (must be 4) */
    uint16_t operation; /* ARP opcode */
    uint8_t sHaddr[6]; /* sender's hardware address */
    uint8_t sInaddr[4]; /* sender's IP address */
    uint8_t tHaddr[6]; /* target's hardware address */
    uint8_t tInaddr[4]; /* target's IP address */
    uint8_t pad[14]; /* pad for min. Ethernet payload (60 bytes) */
} __attribute__((packed)) ;

/**
 * receive arp packet information
 */
struct arpinfo {
	uint32_t ip;
	uint8_t mac[6];
	time_t timestamp;
}__attribute__((packed)) ;

class ARPFilterMember : public IFilterMember {
public:
	ARPFilterMember(const char* name, const char *ifname, const char *filterExpression)
		: IFilterMember(name, ifname, filterExpression) { }
	virtual ~ARPFilterMember() { }

public:
	/**
	 * filter handler
	 */
	void filterHandler(uint8_t* buf, size_t len);

	/**
	 * @brief: for debug. show arp information
	 */
	static void showArpinfo(struct arpinfo *arp);

	/**
	 * @brief: for debug. show arp packet
	 */
	static void showARPPacket(struct arp_message *arp);

	/**
	 * @brief: for debug. show announce data
	 */
	static void showAnnounceData(
			const char* title, uint32_t sender_ipaddr, struct announce_data *anno);

	/**
	 * @brief: check announce ARP packet
	 * sInaddr: ARP source ip address
	 * sHaddr: ARP source hardware address
	 * tInaddr: ARP target ip address
	 * tHaddr: ARP target hardware address
	 * anno: struct announce_data format of ARP padding
	 */
	static bool validAnnounceARP(
			uint8_t *sInaddr, uint8_t *sHaddr,
			uint8_t *tInaddr, uint8_t *tHaddr,
			struct announce_data *anno);

	/**
	 * padding:
	 * 802.3: 18 bytes
	 * 802.1q: 14 bytes
	 *
	 * 4 bytes : magic key (fixed magic key : 0x19830421
	 * 6 bytes : announce mac address
	 * 4 bytes : announce ip address
	 * cf. sender ip address - arp packet target ip address
	 *     caution!!: source ip addres and source hardware address is critical
	 *                ARP caching table of others hosts
	 * ARP source ethernet address be MUST 'sendor mac address'
	 * - part of ARP padding
	 * +--------+--------+--------+--------+
	 * | 0x19   0x83     0x04    0x21 |
	 * +--------+--------+--------+--------+
	 * |             magic            |
	 * +--------+--------+--------+--------+
	 * |          announce    mac     |
	 * +--------+--------+--------+--------+
	 * |   address    |   announce ip |
	 * +--------+--------+--------+--------+
	 * |   address     |
	 * +--------+--------+
	 */
	static bool sendAnnounceARP(
			const char *ifname,
			uint32_t announce_ipaddr, uint8_t *announce_macaddr,
			uint32_t sendder_ipaddr, uint8_t *sender_macaddr);

	static bool sendARPRequest(
			const char *ifname,
		uint32_t sender_ipaddr, uint8_t *sender_macaddr,
		uint32_t target_ipaddr, uint8_t *target_macaddr);
};

} /* namespace kinow */

#endif /* CAPTURE_ARPFILTERMEMBER_H_ */
