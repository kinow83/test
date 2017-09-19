/*
 * WlanUtils.cpp
 *
 *  Created on: 2016. 9. 9.
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <stdint.h>
#include <linux/if_arp.h>
#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>    /* struct sockaddr_ll */
#include <linux/wireless.h>     /* srtuct iwreq */
#include <linux/if_ether.h>
#include <errno.h>
#include <linux/if.h>           /* struct ifr */
#include <linux/if_packet.h>    /* struct sockaddr_ll */
#include <linux/wireless.h>     /* srtuct iwreq */

#include "ieee80211_prism.h"
#include "ieee80211_radiotap.h"
#include "ieee80211_frame.h"
#include "WlanUtils.h"

namespace kinow {

int IEEE80211Freq[][2] = {
	{1, 2412},
	{2, 2417},
	{3, 2422},
	{4, 2427},
	{5, 2432},
	{6, 2437},
	{7, 2442},
	{8, 2447},
	{9, 2452},
	{10, 2457},
	{11, 2462},
	{12, 2467},
	{13, 2472},
	{14, 2484},
	// We could do the math here, but what about 4ghz nonsense?
	// We'll do table lookups for now.
	{36, 5180},
	{37, 5185},
	{38, 5190},
	{39, 5195},
	{40, 5200},
	{41, 5205},
	{42, 5210},
	{43, 5215},
	{44, 5220},
	{45, 5225},
	{46, 5230},
	{47, 5235},
	{48, 5240},
	{52, 5260},
	{53, 5265},
	{54, 5270},
	{55, 5275},
	{56, 5280},
	{57, 5285},
	{58, 5290},
	{59, 5295},
	{60, 5300},
	{64, 5320},
	/* UNET Channel Add by park 2011.05.13 */
	{100, 5500},
	{104, 5520},
	{108, 5540},
	{112, 5560},
	{116, 5580},
	{120, 5600},
	{124, 5620},
	{128, 5640},
	{132, 5660},
	{136, 5680},
	{140, 5700},
	/* End */
	{149, 5745},
	{150, 5750},
	{152, 5760},
	{153, 5765},
	{157, 5785},
	{160, 5800},
	{161, 5805},
	{165, 5825},
	{0, 0}
};

int WlanUtils::getDLT(const char* ifname) {
	int dlt;
	struct ifreq ifr;
	int arp_type;
	int sockfd;

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd < 0) {
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1) {
		close(sockfd);
		return DLT_UNKOWN;
	}

	arp_type = ifr.ifr_hwaddr.sa_family;
	switch (arp_type) {
#ifdef ARPHRD_IEEE80211
	case ARPHRD_IEEE80211:
		dlt = DLT_RADIO;
		break;
#endif

#ifdef ARPHRD_IEEE80211_PRISM
	case ARPHRD_IEEE80211_PRISM:
		dlt = DLT_PRISM;
		break;
#endif

#ifdef ARPHRD_IEEE80211_RADIOTAP
	case ARPHRD_IEEE80211_RADIOTAP:
		dlt = DLT_RADIOTAP;
		break;
#endif
	default:
		dlt = DLT_UNKOWN;
	}

	close(sockfd);
	return dlt;
}

int WlanUtils::FreqTochannel(int freq) {
#if 0
	int x = 0;
	// 80211b frequencies to channels

	while (IEEE80211Freq[x][0] != 0) {
		if (IEEE80211Freq[x][0] == freq) {
			return IEEE80211Freq[x][1];
		}
		x++;
	}
	return freq;
#else
	if (freq < 5000) {
		return ((freq - 2412) / 5) + 1;
	} else {
		return ((freq - 5180) / 5) + 36;
	}
#endif
}

void WlanUtils::showMacfmt(const char* title, const uint8_t* mac, bool newline) {
	printf("%s: "_MAC_FMT_"%s", title, _MAC_FMT_FILL_(mac), newline ? "\n":"");
}

void WlanUtils::showMacfmt64(const char* title, const uint64_t *mac64, bool newline) {
	printf("%s: "_MAC_FMT_"%s",title, _MAC_FMT_FILL_64_(mac64), newline ? "\n":"");
}

void WlanUtils::showFrameType(uint8_t type, uint8_t subtype, bool newline) {
	const char* stype;
	const char* ssubtype;

	switch (type) {
	case packet_management:
		switch (subtype) {
		case packet_sub_association_req:
			ssubtype = "ASSC_REQ"; break;
		case packet_sub_association_resp:
			ssubtype = "ASSC_RES"; break;
		case packet_sub_reassociation_req:
			ssubtype = "REASSC_REQ"; break;
		case packet_sub_reassociation_resp:
			ssubtype = "REASSC_RES"; break;
		case packet_sub_probe_req:
			ssubtype = "PROBE_REQ"; break;
		case packet_sub_probe_resp:
			ssubtype = "PROBE_RES"; break;
		case packet_sub_beacon:
			ssubtype = "BEACON"; break;
		case packet_sub_atim:
			ssubtype = "ATIM"; break;
		case packet_sub_disassociation:
			ssubtype = "DIS_ASSOC"; break;
		case packet_sub_authentication:
			ssubtype = "AUTH"; break;
		case packet_sub_deauthentication:
			ssubtype = "DE_AUTH"; break;
		case packet_sub_action:
			ssubtype = "ACTION"; break;
		}
		stype = "MGNT"; break;

	case packet_control:
		switch(subtype) {
		case packet_sub_block_ack_req:
			ssubtype = "BLK_ACK_REQ"; break;
		case packet_sub_block_ack:
			ssubtype = "BLK_ACK"; break;
		case packet_sub_pspoll:
			ssubtype = "PS_POLL"; break;
		case packet_sub_rts:
			ssubtype = "RTS"; break;
		case packet_sub_cts:
			ssubtype = "CTS"; break;
		case packet_sub_ack:
			ssubtype = "ACK"; break;
		case packet_sub_cf_end:
			ssubtype = "CF_END"; break;
		case packet_sub_cf_end_ack:
			ssubtype = "CF_END_ACK"; break;
		}
		stype = "CNTL"; break;

	case packet_data:
		stype = ssubtype = "DATA"; break;
	}
	printf("[%s:%s]%s", stype, ssubtype, newline?"\n":"");
}

bool WlanUtils::setSockOptBroadcat(int sock) {
	static int on = 1;
	return !setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
}

bool WlanUtils::setPromiscMode(int sock, const char* ifname) {
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) {
		return false;
	}
	ifr.ifr_flags |= IFF_PROMISC;
	if (ioctl(sock, SIOCSIFFLAGS, &ifr) == -1) {
		return false;
	}
	return true;
}

bool WlanUtils::setSockBind(int sock, int proto, const char *ifname) {
	struct ifreq ifr;
	struct sockaddr_ll sll;

	memset(&ifr, 0, sizeof(ifr));

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) {
		return false;
	}

	if(ioctl(sock, SIOCGIFINDEX, &ifr) == -1) {
		return false;
	}
	// bind socket
    bzero(&sll, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(proto);

    if(bind(sock, (struct sockaddr*)&sll, sizeof(sll)) == -1) {
        return false;
    }

    return true;
}

bool WlanUtils::setSockOptAddressReuse(int sock) {
	static int on = 1;
	return !setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

bool WlanUtils::setSockOptBindDevice(int sock, const char *ifname) {
	return !setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, ifname, IFNAMSIZ-1);
}

bool WlanUtils::setSockAttachFilter(int sock, struct sock_fprog *bpf) {
	return !setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, bpf, sizeof(*bpf));
}

int WlanUtils::createSocket(int type, int proto) {
	return socket(PF_PACKET, type, htons(proto));
}

void WlanUtils::socketFilterFree(struct sock_filter_st *sf) {
	if (sf) {
		if (sf->filters) {
			free(sf->filters);
		}
	}
}

struct sock_filter_st*
WlanUtils::socketFilterFactory(const char *ifname, const char *filterExpression) {
	FILE *fp;
	char buf[256];
	char tcpdumpCommand[256];
	char *tok;
	char *last;
	int filterCount = 0;
	struct sock_filter *filters;
	int line = 0;
	struct sock_filter_st* sf;

	sf = (struct sock_filter_st*)malloc(sizeof(struct sock_filter_st));
	memset(sf, 0, sizeof(struct sock_filter_st));

	snprintf(tcpdumpCommand, sizeof(tcpdumpCommand),
			"/usr/sbin/tcpdump -i %s \"%s\" -ddd", ifname, filterExpression);

	printf("%s\n", tcpdumpCommand);

	fp = popen(tcpdumpCommand, "r");
	if (fp == NULL) {
		free(sf);
		return NULL;
	}

	if (fgets(buf, sizeof(buf), fp)) {
		// BPF filter count
		sscanf(buf, "%d", &filterCount);

		if (filterCount == 0) {
			free(sf);
			pclose(fp);
			return NULL;
		}

		filters = (struct sock_filter*)malloc(sizeof(struct sock_filter) * filterCount);

		while (fgets(buf, sizeof(buf), fp) != NULL && (line < filterCount)) {
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
		sf->count = filterCount;
		sf->filters = filters;
		pclose(fp);
		return sf;
	}

	free(sf);
	pclose(fp);
	return NULL;
}

void WlanUtils::socketFilterDebug(struct sock_filter_st* sf) {
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

void WlanUtils::debugHexPacket(const char* title, uint8_t *buf, int len) {
	int i;
	printf("%s (%d)\n", title, len);
	for (i=1; i<=len; i++) {
		printf("%02x ", buf[i-1]);
		if (i%8 == 0)  printf(" ");
		if (i%16 == 0) printf("\n");
	}
	printf("\n");
}

bool WlanUtils::getMacAddress(const char *ifname, uint8_t *mac)
{
	int i;
	struct ifreq ifr;
	uint8_t mymac[6] = { 0, };
	int sock;

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock < 0) {
		return false;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);
	if (ioctl(sock, SIOCGIFHWADDR, &ifr)) {
		close(sock);
		return false;
	}
	for (i = 0; i < 6; ++i) {
		mymac[i] = ifr.ifr_addr.sa_data[i];
	}
	close(sock);

	memcpy(mac, mymac, sizeof(mymac));
	return true;
}

bool WlanUtils::getIpAddress(const char *ifname, uint32_t *ipaddress) {
	int sock;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return false;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);
	if (ioctl(sock, SIOCGIFADDR, &ifr)) {
		*ipaddress = 0;
		close(sock);
		return false;
	}
	close(sock);

	*ipaddress = ((struct sockaddr_in *) (&(ifr.ifr_addr)))->sin_addr.s_addr;
	return true;
}

bool WlanUtils::getNetmask(const char *ifname, uint32_t *netmask) {
	int sock;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return false;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);
	if (ioctl(sock, SIOCGIFNETMASK, &ifr)) {
		*netmask = 0;
		close(sock);
		return false;
	}
	close(sock);

	*netmask = ((struct sockaddr_in *) (&(ifr.ifr_addr)))->sin_addr.s_addr;
	return true;
}



} /* namespace kinow */
