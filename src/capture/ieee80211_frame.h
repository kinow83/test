/*
 * ieee80211_frame.h
 *
 *  Created on: 2013. 12. 17.
 *      Author: root
 */

#ifndef IEEE80211_FRAME_H_
#define IEEE80211_FRAME_H_

#include <stdint.h>
#include <endian.h>

//-----------------------------------------
//
//-----------------------------------------
typedef struct {
#if __BYTE_ORDER == __BIG_ENDIAN
    unsigned short subtype : 4;
    unsigned short type : 2;
    unsigned short version : 2;

    unsigned short order : 1;
    unsigned short wep : 1;
    unsigned short more_data : 1;
    unsigned short power_management : 1;

    unsigned short retry : 1;
    unsigned short more_fragments : 1;
    unsigned short from_ds : 1;
    unsigned short to_ds : 1;
#else
    unsigned short version : 2;
    unsigned short type : 2;
    unsigned short subtype : 4;

    unsigned short to_ds : 1;
    unsigned short from_ds : 1;
    unsigned short more_fragments : 1;
    unsigned short retry : 1;

    unsigned short power_management : 1;
    unsigned short more_data : 1;
    unsigned short wep : 1;
    unsigned short order : 1;
#endif
} __attribute__ ((packed)) FRAME_CONTROL;



//-----------------------------------------
//
//-----------------------------------------
typedef struct {
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned char channel_agility: 1;
	unsigned char pbcc: 1; // Packet Binary Convolutional Coding.
	unsigned char short_preamble: 1;
	unsigned char privacy: 1;
	unsigned char cfpoll_request: 1;
	unsigned char cfpoll_able: 1;
	unsigned char ibss: 1;
	unsigned char ess: 1;

	unsigned char rsvd5: 1; // BYTE : 1;
	unsigned char rsvd4: 1; // BYTE : 1;
	unsigned char dsss_ofdm: 1;
	unsigned char rsvd3: 1; // BYTE : 1;
	unsigned char rsn_enabled: 1;
	unsigned char g_short_slottime: 1;
	unsigned char rsvd2: 1; // BYTE : 1;
	unsigned char rsvd1: 1; // BYTE : 1;
#else
	unsigned char ess: 1;
	unsigned char ibss: 1;
	unsigned char cfpoll_able: 1;
	unsigned char cfpoll_request: 1;
	unsigned char privacy: 1;
	unsigned char short_preamble: 1;
	unsigned char pbcc: 1; // Packet Binary Convolutional Coding.
	unsigned char channel_agility: 1;

	unsigned char rsvd1: 1;
	unsigned char rsvd2: 1;
	unsigned char g_short_slottime: 1;
	unsigned char rsn_enabled: 1;
	unsigned char rsvd3: 1;
	unsigned char dsss_ofdm: 1;
	unsigned char rsvd4: 1;
	unsigned char rsvd5: 1;
#endif
} __attribute__((packed)) CAPABILITY_INFO;



typedef struct {
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned short seqnum:12;
	unsigned short flag:4;
#else
	unsigned short flag:4;
	unsigned short seqnum:12;
#endif
}__attribute__((packed)) SFST;


// packet conversion and extraction utilities
// Packet types, these should correspond to the frame header types
enum ieee_80211_type {
    packet_noise 		= -2,  	// We're too short or otherwise corrupted
    packet_unknown 		= -1, 	// What are we?
    packet_management 	= 0, 	// LLC management
    packet_control 		= 1, 	// Physical layer packets, most drivers can't provide these
    packet_data 		= 2 	// Data frames
};


// Subtypes are a little odd because we re-use values depending on the type
enum ieee_80211_subtype {
		packet_sub_unknown = -1,
		// Management subtypes
		packet_sub_association_req = 0,
		packet_sub_association_resp = 1,
		packet_sub_reassociation_req = 2,
		packet_sub_reassociation_resp = 3,
		packet_sub_probe_req = 4,
		packet_sub_probe_resp = 5,
		packet_sub_beacon = 8,
		packet_sub_atim = 9,
		packet_sub_disassociation = 10,
		packet_sub_authentication = 11,
		packet_sub_deauthentication = 12,
		packet_sub_action = 13, // 2014. kinow
		// Phy subtypes
		packet_sub_block_ack_req = 8,  // 2014. kinow.
		packet_sub_block_ack = 9,  // 2014. kinow.
		packet_sub_pspoll = 10,
		packet_sub_rts = 11,
		packet_sub_cts = 12,
		packet_sub_ack = 13,
		packet_sub_cf_end = 14,
		packet_sub_cf_end_ack = 15,
		// Data subtypes
		packet_sub_data = 0,
		packet_sub_data_cf_ack = 1,
		packet_sub_data_cf_poll = 2,
		packet_sub_data_cf_ack_poll = 3,
		packet_sub_data_null = 4,
		packet_sub_cf_ack = 5,
		packet_sub_cf_ack_poll = 6,
		packet_sub_data_qos_data = 8,
		packet_sub_data_qos_data_cf_ack = 9,
		packet_sub_data_qos_data_cf_poll = 10,
		packet_sub_data_qos_data_cf_ack_poll = 11,
		packet_sub_data_qos_null = 12,
		packet_sub_data_qos_cf_poll_nod = 14,
		packet_sub_data_qos_cf_ack_poll = 15
	};


// distribution directions
enum ieee_80211_disttype {
    distrib_unknown, distrib_from, distrib_to,
    distrib_inter, distrib_adhoc
};

enum channel_band {
    channel_unkown = 0,
    channel_band2,
    channel_band5,
};

// Signalling layer info - what protocol are we seeing data on?
// Not all of these types are currently supported, of course
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

// Packet encoding info - how are packets encoded?
enum phy_encoding_type {
    encoding_unknown,
    encoding_cck,
    encoding_pbcc,
    encoding_ofdm,
	encoding_dynamiccck,
	encoding_gfsk
};

// Turbocell modes
enum turbocell_type {
    turbocell_unknown,
    turbocell_ispbase, // 0xA0
    turbocell_pollbase, // 0x80
    turbocell_nonpollbase, // 0x00
    turbocell_base // 0x40
};

// IAPP crypt enums
enum iapp_type {
    iapp_announce_request = 0,
    iapp_announce_response = 1,
    iapp_handover_request = 2,
    iapp_handover_response = 3
};

enum iapp_pdu {
    iapp_pdu_ssid 		= 0x00,
    iapp_pdu_bssid 		= 0x01,
    iapp_pdu_oldbssid 	= 0x02,
    iapp_pdu_msaddr 	= 0x03,
    iapp_pdu_capability = 0x04,
    iapp_pdu_announceint = 0x05,
    iapp_pdu_hotimeout 	= 0x06,
    iapp_pdu_messageid 	= 0x07,
    iapp_pdu_phytype 	= 0x10,
    iapp_pdu_regdomain 	= 0x11,
    iapp_pdu_channel 	= 0x12,
    iapp_pdu_beaconint 	= 0x13,
    iapp_pdu_ouiident 	= 0x80,
    iapp_pdu_authinfo 	= 0x81
};

enum iapp_cap {
    iapp_cap_forwarding = 0x40,
    iapp_cap_wep = 0x20
};

enum iapp_phy {
    iapp_phy_prop = 0x00,
    iapp_phy_fhss = 0x01,
    iapp_phy_dsss = 0x02,
    iapp_phy_ir = 0x03,
    iapp_phy_ofdm = 0x04
};

enum iapp_dom {
    iapp_dom_fcc 	= 0x10,
    iapp_dom_ic 	= 0x20,
    iapp_dom_etsi 	= 0x30,
    iapp_dom_spain 	= 0x31,
    iapp_dom_france = 0x32,
    iapp_dom_mkk 	= 0x40
};

enum iapp_auth {
    iapp_auth_status 	= 0x01,
    iapp_auth_username 	= 0x02,
    iapp_auth_provname 	= 0x03,
    iapp_auth_rxpkts 	= 0x04,
    iapp_auth_txpkts 	= 0x05,
    iapp_auth_rxbytes 	= 0x06,
    iapp_auth_txbytes 	= 0x07,
    iapp_auth_logintime = 0x08,
    iapp_auth_timelimit = 0x09,
    iapp_auth_vollimit 	= 0x0a,
    iapp_auth_acccycle 	= 0x0b,
    iapp_auth_rxgwords 	= 0x0c,
    iapp_auth_txgwords 	= 0x0d,
    iapp_auth_ipaddr 	= 0x0e,
    iapp_auth_trailer 	= 0xff
};

typedef struct {
    unsigned iapp_version : 8;
    unsigned iapp_type : 8;
} __attribute__ ((packed)) iapp_header;

typedef struct {
    unsigned pdu_type : 8;
    unsigned pdu_len : 16;
} __attribute__ ((packed)) iapp_pdu_header;

// Crypt bitfield
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

typedef struct {
	uint8_t type;
	uint8_t len;
	uint8_t* value;
} __attribute__((packed)) FRAME_TAG;

typedef struct {
	uint8_t type;
	uint16_t len;
	uint8_t value[0];
} __attribute__((packed)) VENDOR_TAG;

#endif /* IEEE80211_FRAME_H_ */
