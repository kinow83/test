/*
 * WlanFilterMember.cpp
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
#include <netinet/ether.h>
#include <stdint.h>
#include <linux/filter.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "ieee80211_prism.h"
#include "ieee80211_radiotap.h"
#include "ieee80211_frame.h"
#include "WlanUtils.h"
#include "WlanFilterMember.h"

namespace kinow {

bool WlanFilterMember::parsingWirelessRadiotap(uint8_t* buf, size_t len, IFilterMember *fm) {
	struct ieee80211_radiotap_header *pRadioHdr;
	uint8_t channel;
	uint16_t channelFreq;
	int8_t signal;
	int8_t noise;
	uint8_t rate;
	int frameLength;
	int fcslength = 0;
	bool debugFlag = false;

	pRadioHdr = (struct ieee80211_radiotap_header *) buf;

	if (len < (int) sizeof(*pRadioHdr)) {
		return false;
	}
	if (pRadioHdr->it_version != 0) {
		fprintf(stderr, "ieee_radiotap.it_version is not 0\n");
		return false;
	}

	// test present flags of radiotap header
	uint8_t* pOrg = (uint8_t*)pRadioHdr + sizeof(struct ieee80211_radiotap_header);
	uint8_t* pRadioData = pOrg;
	{
		uint32_t presentFlag = pRadioHdr->it_present;

		if (presentFlag & (1<<IEEE80211_RADIOTAP_TSFT)) {
			if (debugFlag) printf("+TSFT ");
			pRadioData += sizeof(uint64_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_FLAGS)) {
			if (debugFlag) printf("+FLAGS ");
			uint8_t flags = *pRadioData;
			if (flags & (IEEE80211_RADIOTAP_F_BAD_FCS)) {
				return false;
			}
			if (flags & (1<<IEEE80211_RADIOTAP_F_FCS)) {
				fcslength = 4;
			}
			pRadioData += sizeof(uint8_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_RATE)) {
			if (debugFlag) printf("+RATE ");
			rate = (*pRadioData) / 2;
			pRadioData += sizeof(uint8_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_CHANNEL)) {
			if (debugFlag) printf("+CHANNEL ");
			channelFreq = *(uint16_t*)pRadioData;
			channel = WlanUtils::FreqTochannel(channelFreq);
			pRadioData += sizeof(uint16_t);

			uint16_t channelFlags = *(uint16_t*)pRadioData;
			pRadioData += sizeof(uint16_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_FHSS)) {
			if (debugFlag) printf("+FHSS ");
			pRadioData += sizeof(uint16_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_DBM_ANTSIGNAL)) {
			if (debugFlag) printf("+SIGNAL ");
			signal = *pRadioData;
			pRadioData += sizeof(uint8_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_DBM_ANTNOISE)) {
			if (debugFlag) printf("+NOISE ");
			noise = *pRadioData;
			pRadioData += sizeof(uint8_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_LOCK_QUALITY)) {
			pRadioData += sizeof(uint16_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_TX_ATTENUATION)) {
			pRadioData += sizeof(uint16_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_DB_TX_ATTENUATION)) {
			pRadioData += sizeof(uint16_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_DBM_TX_POWER)) {
			pRadioData += sizeof(uint8_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_ANTENNA)) {
			if (debugFlag) printf("+ANTENNA ");
			pRadioData += sizeof(uint8_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_DB_ANTSIGNAL)) {
			pRadioData += sizeof(uint8_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_DB_ANTNOISE)) {
			pRadioData += sizeof(uint8_t);
		}
		if (presentFlag & (1<<IEEE80211_RADIOTAP_RX_FLAGS)) {
			if (debugFlag) printf("+RX_FLAGS ");
			pRadioData += sizeof(uint16_t);
		}
		// TODO: Who one fill next flags.
		// from CHANNEL_PLUS to VENDOR_NS_NEXT
		if (debugFlag) printf("\n");
	}

	frameLength = len - pRadioHdr->it_len - fcslength;
//	debugHex("packet", buf, frameLength);

//	printf("ch: %d, rate: %d, signal: %d, noise: %d, framelen: %d, dist:%ld\n",
//			channel, rate, signal, noise, frameLength, pRadioData-pOrg);

	printf("{%s} ", fm->getName());
	printf("ch: %d(%d), rate: %d, rssi: %d, len: %d",
			channel, channelFreq, rate, signal, frameLength);


	uint8_t *pData = pRadioData;
	FRAME_CONTROL *fc = (FRAME_CONTROL *)pData;
	{
//		debugHex("FC", (uint8_t*)fc, sizeof(FRAME_CONTROL));
		uint8_t version = fc->version;
		uint8_t type = fc->type;
		uint8_t subtype = fc->subtype;
//		printf("ver:%d, type:%d, subtype:%d fromds:%d, tods:%d\n", version, type, subtype, fc->from_ds, fc->to_ds);
		pData += sizeof(FRAME_CONTROL);

		uint16_t duration = ntohs(*(uint16_t*)pData); pData += sizeof(uint16_t);
		WlanUtils::showFrameType(type, subtype, false);
		WlanUtils::showMacfmt(" Receiver", pData, false); pData += 6;
		WlanUtils::showMacfmt(" Transmit", pData, false); pData += 6;
		WlanUtils::showMacfmt(" Source", pData, false);   pData += 6;
	}
	printf("\n");
	return true;
}

bool WlanFilterMember::parsingEthernet(uint8_t* buf, size_t len) {
	struct ip *iph;
	struct tcphdr *tcph;
	struct ether_header *ep;
	unsigned short ether_type;
	uint8_t* packet;

//	debugHex(getName(), buf, len);

	packet = buf;
	ep = (struct ether_header *) packet;
	packet += sizeof(struct ether_header);
	ether_type = ntohs(ep->ether_type);

	if (ether_type == ETHERTYPE_IP) {
		// IP 헤더에서 데이타 정보를 출력한다.
		iph = (struct ip *) packet;
		printf("%s -> ", inet_ntoa(iph->ip_src));
		printf("%s", inet_ntoa(iph->ip_dst));

		if (iph->ip_p == IPPROTO_TCP) {
			tcph = (struct tcphdr *) (packet + iph->ip_hl * 4);
			printf("[%d -> ", ntohs(tcph->source));
			printf("%d]", ntohs(tcph->dest));
		}
		printf("\n\n");
	}
	return true;
}

void WlanFilterMember::filterHandler(uint8_t* buf, size_t len) {
	static int dlt = WlanUtils::getDLT(getIfname());
	switch (dlt) {
	case DLT_RADIOTAP:
		parsingWirelessRadiotap(buf, len, this);
		break;
	default:
		fprintf(stderr, "unsupport DLT: %d\n", dlt);
		return;
	}
}

} /* namespace kinow */
