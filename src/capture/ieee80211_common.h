/*
 * ieee80211_common.h
 *
 *  Created on: 2013. 12. 18.
 *      Author: root
 */

#ifndef IEEE80211_COMMON_H_
#define IEEE80211_COMMON_H_


enum channel_band {
	channel_unkown = 0,
	channel_band2,
	channel_band5,
};

enum phy_carrier_type {
    carrier_unknown,
    carrier_80211b,
    carrier_80211bplus,
    carrier_80211a,
    carrier_80211g,
    carrier_80211fhss,
    carrier_80211dsss,
	carrier_80211n20,
	carrier_80211n40,
	carrier_80211ac80,
	carrier_80211ac160
};

#if 0
static const char *phy_carrier_string [] = {
	"unknown",
	"80211b",
	"80211bplus",
	"80211a",
	"80211g",
	"80211fhss",
	"80211dsss",
	"80211n20",
	"80211n40",
	"80211ac80",
	"80211ac160"
};
#endif


enum crypt_type {
	crypt_none = 0,
	crypt_unknown = 1,
	crypt_wep = (1 << 1),
	crypt_layer3 = (1 << 2),
	// Derived from WPA headers
	crypt_wep40 = (1 << 3),
	crypt_wep104 = (1 << 4),
	crypt_tkip = (1 << 5),
	crypt_wpa = (1 << 6),
	crypt_psk = (1 << 7),
	crypt_aes_ocb = (1 << 8),
	crypt_aes_ccm = (1 << 9),
	// Derived from data traffic
	crypt_leap = (1 << 10),
	crypt_ttls = (1 << 11),
	crypt_tls = (1 << 12),
	crypt_peap = (1 << 13),
	crypt_isakmp = (1 << 14),
    crypt_pptp = (1 << 15),
	crypt_fortress = (1 << 16),
	crypt_keyguard = (1 << 17),
	crypt_unknown_nonwep = (1 << 18),
};



#endif /* IEEE80211_COMMON_H_ */
