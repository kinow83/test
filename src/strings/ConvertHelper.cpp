/*
 * ConvertHelper.cpp
 *
 *  Created on: 2016. 10. 24.
 *      Author: root
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ConvertHelper.h"
#include "test_endian.h"

namespace kinow {

static bool char2Hex(char in, unsigned char *byte)
{
	if (in >= '0' && in <= '9') {
		*byte = in - '0';
		return true;
	}
	if (in >= 'A' && in <= 'Z') {
		*byte = in - 'A' + 0x0A;
		return true;
	}
	if (in >= 'a' && in <= 'z') {
		*byte = in - 'a' + 0x0a;
		return true;
	}
	return false;
}

bool ConvertHelper::string2mac(const char* str, unsigned char* bytes6)
{
	int len;
	int i = 0, k = 0;
	unsigned char ch1, ch2, hex;

	if (!str) return false;

	len = strlen(str);
	for (i=0; i<len && k<6; i++) {
		if (str[i] != '.' && str[i] != ':') {
			if (	char2Hex(str[i  ], &ch1) &&
					char2Hex(str[i+1], &ch2)) {
				bytes6[k++] = (ch1 << 4) | ch2;
				i++;
			}
		}
	}

	if (k != 6) {
		return false;
	}
	return true;
}

bool ConvertHelper::string2ipv4(const char* str, uint32_t* ipv4)
{
	unsigned char a, b, c, d;

	if (!str) return false;

	if (IS_BIG_ENDIAN) {
		if (sscanf(str, "%hhu.%hhu.%hhu.%hhu", &d, &c, &b, &a) != 4)
			return false;
	} else {
		if (sscanf(str, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d) != 4)
			return false;
	}
	*ipv4 = ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
	return true;
}

const char* ConvertHelper::mac2string(char* str, size_t strlen, unsigned char* bytes6, bool uppercase)
{
	static const char *mac_uppercase = "%02X:%02X:%02X:%02X:%02X:%02X";
	static const char *mac_lowercase = "%02x:%02x:%02x:%02x:%02x:%02x";
	const char *fmt = uppercase ? mac_uppercase : mac_lowercase;

	snprintf(str, strlen, fmt,
			bytes6[0], bytes6[1], bytes6[2],
			bytes6[3], bytes6[4], bytes6[5]);
	return str;
}

const char* ConvertHelper::ipv42string(char* str, size_t strlen, uint32_t ipv4)
{
	unsigned char *c = (unsigned char*)&ipv4;
	if (IS_BIG_ENDIAN) {
		snprintf(str, strlen, "%d.%d.%d.%d", c[0], c[1], c[2], c[3]);
	} else {
		snprintf(str, strlen, "%d.%d.%d.%d", c[3], c[2], c[1], c[0]);
	}
	return str;
}

} /* namespace kinow */

#if 0

using namespace kinow;

int main(int argc, char **argv)
{
	char ip[32];
	uint32_t ipv4;
	bool b;

	b = ConvertHelper::string2ipv4("10.20.30.40", &ipv4);
	printf("%u\n", ipv4);

	ConvertHelper::ipv42string(ip, sizeof(ip), ipv4);
	printf("%s\n", ip);

	unsigned char mac[6];
	const char *mac_str = "3A:aa:F1:22:dd:fF";
	printf("mac = %s\n", mac_str);
	ConvertHelper::string2mac(mac_str, mac);
	printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	return 0;
}
#endif
