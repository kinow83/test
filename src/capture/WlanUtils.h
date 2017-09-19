/*
 * WlanUtils.h
 *
 *  Created on: 2016. 9. 9.
 *      Author: root
 */

#ifndef CAPTURE_WLANUTILS_H_
#define CAPTURE_WLANUTILS_H_

#include <stdint.h>
#include <string.h>
#include <linux/if_packet.h>    /* struct sockaddr_ll */
#include <linux/filter.h>
#include <stdint.h>

namespace kinow {

#ifndef MAC_BCAST_ADDR
 #define MAC_BCAST_ADDR ((uint8_t *)"\xff\xff\xff\xff\xff\xff")
#endif

#ifndef _MAC_FMT_
 #define _MAC_FMT_ "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#ifndef _MAC7_FMT_
 #define _MAC7_FMT_ "%02x:%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#ifndef _MAC_FMT_FILL_
 #define _MAC_FMT_FILL_(x) \
		((uint8_t*)(x))[0], \
		((uint8_t*)(x))[1], \
		((uint8_t*)(x))[2], \
		((uint8_t*)(x))[3], \
		((uint8_t*)(x))[4], \
		((uint8_t*)(x))[5]
#endif

#ifndef _MAC7_FMT_FILL_
 #define _MAC7_FMT_FILL_(x) \
		((uint8_t*)(x))[0], \
		((uint8_t*)(x))[1], \
		((uint8_t*)(x))[2], \
		((uint8_t*)(x))[3], \
		((uint8_t*)(x))[4], \
		((uint8_t*)(x))[5], \
		((uint8_t*)(x))[6]
#endif

#ifndef _MAC_FMT_FILL_64_
 #define _MAC_FMT_FILL_64_(x) \
		((uint8_t*)(&x))[5], \
		((uint8_t*)(&x))[4], \
		((uint8_t*)(&x))[3], \
		((uint8_t*)(&x))[2], \
		((uint8_t*)(&x))[1], \
		((uint8_t*)(&x))[0]
#endif

#ifndef _MAC7_FMT_FILL_64_
 #define _MAC7_FMT_FILL_64_(x) \
		((uint8_t*)(&x))[6], \
		((uint8_t*)(&x))[5], \
		((uint8_t*)(&x))[4], \
		((uint8_t*)(&x))[3], \
		((uint8_t*)(&x))[2], \
		((uint8_t*)(&x))[1], \
		((uint8_t*)(&x))[0]
#endif

#ifndef _IS_ZERO_MAC_
 #define _IS_ZERO_MAC_(x) (( \
		((uint8_t*)(x))[0] | \
		((uint8_t*)(x))[1] | \
		((uint8_t*)(x))[2] | \
		((uint8_t*)(x))[3] | \
		((uint8_t*)(x))[4] | \
		((uint8_t*)(x))[5]) == 0)
#endif

#ifndef _IS_ZERO_IP_
 #define _IS_ZERO_IP_(x) (( \
		((uint8_t*)(x))[0] | \
		((uint8_t*)(x))[1] | \
		((uint8_t*)(x))[2] | \
		((uint8_t*)(x))[3]) == 0)
#endif

#ifndef _IS_BCAST_MAC_
 #define _IS_BCAST_MAC_(x) (( \
		((uint8_t*)(x))[0] & \
		((uint8_t*)(x))[1] & \
		((uint8_t*)(x))[2] & \
		((uint8_t*)(x))[3] & \
		((uint8_t*)(x))[4] & \
		((uint8_t*)(x))[5]) == 0xFF)
#endif

#ifndef _IS_BCAST_IP_
 #define _IS_BCAST_IP_(x) (( \
		((uint8_t*)(x))[0] & \
		((uint8_t*)(x))[1] & \
		((uint8_t*)(x))[2] & \
		((uint8_t*)(x))[3]) == 0xFF)
#endif

#ifndef _MAC_COPY_
 #define _MAC_COPY_(t, s) \
    memcpy((t), (s), 6)
#endif

#ifndef _MAC_CMP_
 #define _MAC_CMP_(t, s) \
    ((memcmp((t), (s), 6) == 0) ? 1 : 0)
#endif

#ifndef _IP_FMT_
 #define _IP_FMT_ "%d.%d.%d.%d"
#endif

#ifndef _IP_FMT_FILL_
 #define _IP_FMT_FILL_(x) \
	((uint8_t*)(&(x)))[0], ((uint8_t*)(&(x)))[1], ((uint8_t*)(&(x)))[2], ((uint8_t*)(&(x)))[3]
#endif

#ifndef IS_MCAST_IP
 #define IS_MCAST_IP(x) \
	((((uint8_t*)(&(x)))[0] >> 4) == 0x0E)
#endif


enum wifi_dlt {
	DLT_UNKOWN              = 0,
	DLT_PRISM               = 1,
	DLT_RADIOTAP            = 2,
	DLT_RADIO               = 3,
};

struct sock_filter_st {
	uint32_t count;
	struct sock_filter* filters;
};

class WlanUtils {
private:
	WlanUtils() {}
	virtual ~WlanUtils() {}

public:
	static int getDLT(const char* ifname);
	static int FreqTochannel(int in_chan);
	static void showMacfmt(const char* title, const uint8_t* mac, bool newline);
	static void showMacfmt64(const char* title, const uint64_t *mac64, bool newline);
	static void showFrameType(uint8_t type, uint8_t subtype, bool newline);
	static int createSocket(int type, int proto);
	static bool setSockOptAddressReuse(int sock);
	static bool setSockOptBindDevice(int sock, const char *ifname);
	static bool setSockBind(int sock, int proto, const char *ifname);
	static bool setSockAttachFilter(int sock, struct sock_fprog *bpf);
	static bool setPromiscMode(int sock, const char* ifname);
	static bool setSockOptBroadcat(int sock);
	static void debugHexPacket(const char* title, uint8_t *buf, int len);
	static void socketFilterFree(struct sock_filter_st *sf);
	static struct sock_filter_st* socketFilterFactory(const char *ifname, const char *filterExpression);
	static void socketFilterDebug(struct sock_filter_st* sf);
	static bool getMacAddress(const char *ifname, uint8_t *mac);
	static bool getIpAddress(const char *ifname, uint32_t *ipaddress);
	static bool getNetmask(const char *ifname, uint32_t *netmask);
};

} /* namespace kinow */

#endif /* CAPTURE_WLANUTILS_H_ */
