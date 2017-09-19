/*
 * ieee80211_prism.h
 *
 *  Created on: 2013. 12. 17.
 *      Author: root
 */

/* INFORMATION ON PRISM HEADERS
 *
 * Its a bit hard to get reliable information on Prism headers
 * The following information has been "gleamed" from examining
 * other drivers
 *
 * Endianness:  Prism headers are in "host" order
 *
 * did:     values are #defined below
 *
 * status:   0 indicates that the parameter is supplied by the driver
 *      1 indicates that the driver doesn't supply the parameter.
 *      if not supplied it should be ignored
 *
 * len:     1-4  This is the number of bytes in the data field
 *            In all the drivers I have looked at, this value has always been 4
 *
 * data:      The actual value - if all bits are not used they should be zeroed
 *
 *
 * The Broadcom driver PRISM header supplies:
 *
 * hosttime:  In jiffies - for our system this is in 10ms units
 *
 * mactime:     In micro-seconds - not much use to us, we want it in milliseconds
 *      a 32 bit usec timer will will role over in just over an hour.
 *      Drivers appear to use a 64bit counter to hold mactime internal
 *      the then fill the prism header with the lower 32 bits
 *
 * channel: Not Supplied
 *
 * rssi:     Appears to be a signed dbm value
 *
 * sq:     Signal quality - I think that this is something to do with the number
 *            of errors received
 *
 * signal:     Should be the signal strength in dbm - but doesn't appear to be.
 *      I have seen some drivers use a value of 100-(signal in dbm) to
 *      provide a positive integer.  We are getting a value of "2"
 *
 * noise:     Not Supplied - Should be signed dbm value
 *
 * rate:     Appears to be in units/multiples of 500Khz
 *
 * istx:     Not Supplied  - 0 = rx packet, 1 = tx packet
 *
 * frmlen:     Length of the following frame in bytes
 *
 *
 */

#ifndef IEEE80211_PRISM_H_
#define IEEE80211_PRISM_H_


#include <linux/types.h>
#include <stdint.h>

#define DNAMELEN 16 // Device name length

#define PRISM_MSGCODE           0x0041 // Monitor Frame
#define PRISM_HEADER_LENGTH     144             /* Default Prism Header Length */
#define PRISM_DID_HOSTTIME      0x00010044      /* Host time element */
#define PRISM_DID_MACTIME       0x00020044      /* Mac time element */
#define PRISM_DID_CHANNEL       0x00030044      /* Channel element */
#define PRISM_DID_RSSI          0x00040044      /* RSSI element */
#define PRISM_DID_SQ            0x00050044      /* SQ element */
#define PRISM_DID_SIGNAL        0x00060044      /* Signal element */
#define PRISM_DID_NOISE         0x00070044      /* Noise element */
#define PRISM_DID_RATE          0x00080044      /* Rate element */
#define PRISM_DID_ISTX          0x00090044      /* Is Tx frame */
#define PRISM_DID_FRMLEN        0x000A0044      /* Frame length */


#define PRISM_STATUS_OK 0 // Prism Status: the associated prism_value is supplied
#define PRISM_STATUS_NO_VALUE 1 // Prism Status: the associated prism_value is NOT supplied


struct prism_value
{
    uint32_t did; // This has a different ID for each parameter
    uint16_t status; // 0 = set;  1 = not set (yes - not what you expected)
    uint16_t len; // length of the data (u32) used 0-4
    uint32_t data; // The data value
} __attribute__ ((packed));

struct prism_header
{
    uint32_t msgcode;             // = PRISM_MSGCODE
    uint32_t msglen;              // The length of the entire header - usually 144 bytes = 0x90
    char devname[DNAMELEN];       // The name of the device that captured this packet
    struct prism_value hosttime;  // This is measured in jiffies - I think
    struct prism_value mactime;   // This is a truncated microsecond timer,
                                  // we get the lower 32 bits of a 64 bit value
    struct prism_value channel;
    struct prism_value rssi;
    struct prism_value sq;
    struct prism_value signal;
    struct prism_value noise;
    struct prism_value rate;
    struct prism_value istx;
    struct prism_value frmlen;
    char   dot_11_header[];
} __attribute__ ((packed));



#endif /* IEEE80211_PRISM_H_ */
