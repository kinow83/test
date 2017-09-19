/*
 * ieee80211_radiotap.h
 *
 *  Created on: 2013. 12. 18.
 *      Author: root
 */

#ifndef IEEE80211_RADIOTAP_H_
#define IEEE80211_RADIOTAP_H_

#include <stdint.h>

/* Kluge the radiotap linktype for now if we don't have it */
#ifndef LNX_ARPHRD_IEEE80211_RADIOTAP
#define LNX_ARPHRD_IEEE80211_RADIOTAP 803
#endif

/* Radiotap header version (from official NetBSD feed) */
#define IEEE80211RADIOTAP_VERSION	"1.5"
/* Base version of the radiotap packet header data */
#define PKTHDR_RADIOTAP_VERSION		0

/* A generic radio capture format is desirable. There is one for
 * Linux, but it is neither rigidly defined (there were not even
 * units given for some fields) nor easily extensible.
 *
 * I suggest the following extensible radio capture format. It is
 * based on a bitmap indicating which fields are present.
 *
 * I am trying to describe precisely what the application programmer
 * should expect in the following, and for that reason I tell the
 * units and origin of each measurement (where it applies), or else I
 * use sufficiently weaselly language ("is a monotonically nondecreasing
 * function of...") that I cannot set false expectations for lawyerly
 * readers.
 */

/* XXX tcpdump/libpcap do not tolerate variable-length headers,
 * yet, so we pad every radiotap header to 64 bytes. Ugh.
 */
#define IEEE80211_RADIOTAP_HDRLEN	64

/* The radio capture header precedes the 802.11 header. */
struct ieee80211_radiotap_header {
	uint8_t	it_version;	/* Version 0. Only increases
					 * for drastic changes,
					 * introduction of compatible
					 * new fields does not count.
					 */
	uint8_t	it_pad;
	uint16_t       it_len;         /* length of the whole
					 * header in bytes, including
					 * it_version, it_pad,
					 * it_len, and data fields.
					 */
	uint32_t       it_present;     /* A bitmap telling which
					 * fields are present. Set bit 31
					 * (0x80000000) to extend the
					 * bitmap by another 32 bits.
					 * Additional extensions are made
					 * by setting bit 31.
					 */
};

/* Name                                 Data type       Units
 * ----                                 ---------       -----
 *
 * IEEE80211_RADIOTAP_TSFT              u_int64_t       microseconds
 *
 *      Value in microseconds of the MAC's 64-bit 802.11 Time
 *      Synchronization Function timer when the first bit of the
 *      MPDU arrived at the MAC. For received frames, only.
 *
 * IEEE80211_RADIOTAP_CHANNEL           2 x u_int16_t   MHz, bitmap
 *
 *      Tx/Rx frequency in MHz, followed by flags (see below).
 *
 * IEEE80211_RADIOTAP_FHSS              u_int16_t       see below
 *
 *      For frequency-hopping radios, the hop set (first byte)
 *      and pattern (second byte).
 *
 * IEEE80211_RADIOTAP_RATE              u_int8_t        500kb/s
 *
 *      Tx/Rx data rate
 *
 * IEEE80211_RADIOTAP_DBM_ANTSIGNAL     int8_t          decibels from
 *                                                      one milliwatt (dBm)
 *
 *      RF signal power at the antenna, decibel difference from
 *      one milliwatt.
 *
 * IEEE80211_RADIOTAP_DBM_ANTNOISE      int8_t          decibels from
 *                                                      one milliwatt (dBm)
 *
 *      RF noise power at the antenna, decibel difference from one
 *      milliwatt.
 *
 * IEEE80211_RADIOTAP_DB_ANTSIGNAL      u_int8_t        decibel (dB)
 *
 *      RF signal power at the antenna, decibel difference from an
 *      arbitrary, fixed reference.
 *
 * IEEE80211_RADIOTAP_DB_ANTNOISE       u_int8_t        decibel (dB)
 *
 *      RF noise power at the antenna, decibel difference from an
 *      arbitrary, fixed reference point.
 *
 * IEEE80211_RADIOTAP_LOCK_QUALITY      u_int16_t       unitless
 *
 *      Quality of Barker code lock. Unitless. Monotonically
 *      nondecreasing with "better" lock strength. Called "Signal
 *      Quality" in datasheets.  (Is there a standard way to measure
 *      this?)
 *
 * IEEE80211_RADIOTAP_TX_ATTENUATION    u_int16_t       unitless
 *
 *      Transmit power expressed as unitless distance from max
 *      power set at factory calibration.  0 is max power.
 *      Monotonically nondecreasing with lower power levels.
 *
 * IEEE80211_RADIOTAP_DB_TX_ATTENUATION u_int16_t       decibels (dB)
 *
 *      Transmit power expressed as decibel distance from max power
 *      set at factory calibration.  0 is max power.  Monotonically
 *      nondecreasing with lower power levels.
 *
 * IEEE80211_RADIOTAP_DBM_TX_POWER      int8_t          decibels from
 *                                                      one milliwatt (dBm)
 *
 *      Transmit power expressed as dBm (decibels from a 1 milliwatt
 *      reference). This is the absolute power level measured at
 *      the antenna port.
 *
 * IEEE80211_RADIOTAP_FLAGS             u_int8_t        bitmap
 *
 *      Properties of transmitted and received frames. See flags
 *      defined below.
 *
 * IEEE80211_RADIOTAP_ANTENNA           u_int8_t        antenna index
 *
 *      Unitless indication of the Rx/Tx antenna for this packet.
 *      The first antenna is antenna 0.
 *
 * IEEE80211_RADIOTAP_FCS           	u_int32_t       data
 *
 *	FCS from frame in network byte order.
 */
enum ieee80211_radiotap_type {
	IEEE80211_RADIOTAP_TSFT = 0, // 8byte (u_int64_t)
	IEEE80211_RADIOTAP_FLAGS = 1, // 1byte (u_int8_t)
	IEEE80211_RADIOTAP_RATE = 2, // 1byte (u_int8_t)
	IEEE80211_RADIOTAP_CHANNEL = 3, // 2byte * 2 (u_int16_t) // with channel flag
	IEEE80211_RADIOTAP_FHSS = 4, // 2byte (u_int16_t)
	IEEE80211_RADIOTAP_DBM_ANTSIGNAL = 5, // 1byte (int8_t)
	IEEE80211_RADIOTAP_DBM_ANTNOISE = 6, // 1byte (int8_t)
	IEEE80211_RADIOTAP_LOCK_QUALITY = 7, // 2byte (u_int16_t)
	IEEE80211_RADIOTAP_TX_ATTENUATION = 8,  // 2byte (u_int16_t)
	IEEE80211_RADIOTAP_DB_TX_ATTENUATION = 9, // 2byte (u_int16_t)
	IEEE80211_RADIOTAP_DBM_TX_POWER = 10, // 1byte (int8_t)
	IEEE80211_RADIOTAP_ANTENNA = 11, // 1byte (u_int8_t)
	IEEE80211_RADIOTAP_DB_ANTSIGNAL = 12, // 1byte (u_int8_t)
	IEEE80211_RADIOTAP_DB_ANTNOISE = 13, // 1byte (u_int8_t)
	IEEE80211_RADIOTAP_RX_FLAGS = 14, // 2byte (u_int16_t)
	/* empty 15 ~ 17 */
	IEEE80211_RADIOTAP_CHANNEL_PLUS = 18,
	IEEE80211_RADIOTAP_MCS_INFORMATION = 19,
	IEEE80211_RADIOTAP_A_MPDU_STATUS = 20,
	IEEE80211_RADIOTAP_VHT_INFORMATION = 21,
	/* reserved 22 ~ 28 */
	IEEE80211_RADIOTAP_RADIOTAP_NS_NEXT = 29,
	IEEE80211_RADIOTAP_VENDOR_NS_NEXT = 30,
	IEEE80211_RADIOTAP_EXT = 31,
};

/* Channel flags. */
#define	IEEE80211_CHAN_TURBO	0x0010	/* Turbo channel */
#define	IEEE80211_CHAN_CCK	0x0020	/* CCK channel */
#define	IEEE80211_CHAN_OFDM	0x0040	/* OFDM channel */
#define	IEEE80211_CHAN_2GHZ	0x0080	/* 2 GHz spectrum channel. */
#define	IEEE80211_CHAN_5GHZ	0x0100	/* 5 GHz spectrum channel */
#define	IEEE80211_CHAN_PASSIVE	0x0200	/* Only passive scan allowed */
#define	IEEE80211_CHAN_DYN	0x0400	/* Dynamic CCK-OFDM channel */
#define	IEEE80211_CHAN_GFSK	0x0800	/* GFSK channel (FHSS PHY) */

/* For IEEE80211_RADIOTAP_FLAGS */
#define	IEEE80211_RADIOTAP_F_CFP	0x01	/* sent/received
						 * during CFP
						 */
#define	IEEE80211_RADIOTAP_F_SHORTPRE	0x02	/* sent/received
						 * with short
						 * preamble
						 */
#define	IEEE80211_RADIOTAP_F_WEP	0x04	/* sent/received
						 * with WEP encryption
						 */
#define	IEEE80211_RADIOTAP_F_FRAG	0x08	/* sent/received
						 * with fragmentation
						 */
#define	IEEE80211_RADIOTAP_F_FCS	0x10	/* frame includes FCS */
#define	IEEE80211_RADIOTAP_F_DATAPAD	0x20	/* frame has padding between
						 * 802.11 header and payload
						 * (to 32-bit boundary)
						 */
#define	IEEE80211_RADIOTAP_F_BAD_FCS	0x40
#define	IEEE80211_RADIOTAP_F_SHORT_GI	0x80

/* Ugly macro to convert literal channel numbers into their mhz equivalents
 * There are certianly some conditions that will break this (like feeding it '30')
 * but they shouldn't arise since nothing talks on channel 30. */
#define ieee80211chan2mhz(x) \
	(((x) <= 14) ? \
	(((x) == 14) ? 2484 : ((x) * 5) + 2407) : \
	((x) + 1000) * 5)


/*
struct wl_radiotap_header {
	struct ieee80211_radiotap_header    ieee_radiotap;
	uint32_t       tsft_h;
	uint32_t       tsft_l;
	uint8_t        flags;
	uint8_t        rate;
	uint16_t       channel_freq;
	uint16_t       channel_flags;
	uint8_t        signal;
	uint8_t        noise;
	uint8_t        antenna;
} __attribute__((__packed__));
*/

#endif /* IEEE80211_RADIOTAP_H_ */
