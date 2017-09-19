/*
 * CaptureARP.cpp
 *
 *  Created on: 2016. 9. 23.
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
#include <getopt.h>

#include "WlanUtils.h"
#include "CaptureARP.h"
#include "Options.h"
#include "StringTok.h"
#include "ConvertHelper.h"

namespace kinow {

} /* namespace kinow */

using namespace kinow;

/*

g++ ../strings/ConvertHelper.cpp ../strings/StringTok.cpp CaptureARP.cpp ARPFilterMember.cpp IFilterMember.cpp ../daemon/Options.cpp WlanFilterMember.cpp WlanUtils.cpp CaptureMember.cpp -o capture  -I ../error/ -I . -I ../daemon/ -I ../strings/

# ./capture -i enp3s0 -m s -d "0.0.0.0, 00:00:00:00:00:00, 10.10.200.141, 00:00:00:00:00:00"
 */

int main(int argc, char **argv) {
	Options options(argc, argv);
	char errbuf[64] = {0, };
	uint8_t mymac[6] = {0, };
	uint32_t myip = 0;
	struct in_addr addr;
	const char *mode = NULL;
	const char *ifname = NULL;
	bool isAnnounce = false;

	options.addOption('m', "mode[s: send, r: recv]",
			Options::REQUIRE_OPT|Options::REQUIRE_OPT_VALUE);
	options.addOption('i', "interface",
			Options::REQUIRE_OPT|Options::REQUIRE_OPT_VALUE);
	options.addOption('a', "announce");
	options.addOption('d',
			"arp send extra information [sender ip, sender mac, target ip, target mac]",
			Options::REQUIRE_OPT_VALUE);
	options.addOptionHelp();

	if (options.validOptions(errbuf, sizeof(errbuf)) == false) {
		fprintf(stderr, "%s\n", errbuf);
		exit(1);
	}

	ifname = options.optionValue('i');
	mode = options.optionValue('m');
	isAnnounce = options.isOption('a');

	if (mode[0] == 's') {
//		printf("isAnnounce = %d\n", isAnnounce);
		if (isAnnounce) {
			WlanUtils::getIpAddress(ifname, &myip);
			WlanUtils::getMacAddress(ifname, mymac);
			inet_aton("10.10.200.187", &addr);
			uint32_t announce_ip = addr.s_addr;
			printf("announce_ip=%u\n", announce_ip);
			uint8_t announce_mac[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
			ARPFilterMember::sendAnnounceARP(ifname,
					announce_ip, announce_mac,
					myip, mymac);
		} else {
			const char *dOption = options.optionValue('d');
			uint32_t sender_ip = 0;
			uint32_t target_ip = 0;
			uint8_t sender_mac[6];
			uint8_t target_mac[6];

			if (dOption == NULL) {
				options.help();
				exit(-1);
			}

			StringTok stok(dOption, ",", true);

			if (stok.size() != 4) {
				options.help();
				exit(-1);
			}

			printf("sender ip  = %s\n", stok[0]);
			printf("target ip  = %s\n", stok[1]);
			printf("sender mac = %s\n", stok[2]);
			printf("target mac = %s\n", stok[3]);

			ConvertHelper::string2ipv4(stok[0], &sender_ip);
			ConvertHelper::string2mac(stok[1], sender_mac);
			ConvertHelper::string2ipv4(stok[2], &target_ip);
			ConvertHelper::string2mac(stok[3], target_mac);

			WlanUtils::debugHexPacket("sender ip", (unsigned char*)&sender_ip, 4);
			WlanUtils::debugHexPacket("sender mac", sender_mac, 6);
			WlanUtils::debugHexPacket("target ip", (unsigned char*)&target_ip, 4);
			WlanUtils::debugHexPacket("target mac", target_mac, 6);

			ARPFilterMember::sendARPRequest(ifname,
					htonl(sender_ip), sender_mac,
					htonl(target_ip), target_mac);
		}
	}
	else
	if (mode[0] == 'r') {
		CaptureMember *capture = new CaptureARP("ARP CAPTURE");
		IFilterMember *arpFilterMember = new ARPFilterMember("ARP", ifname, "arp");
		capture->addFilterMember(arpFilterMember);
		capture->capture();
		delete capture;
	}
}
