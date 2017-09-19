/*
 * CaptureARP.h
 *
 *  Created on: 2016. 9. 23.
 *      Author: root
 */

#ifndef CAPTURE_CAPTUREARP_H_
#define CAPTURE_CAPTUREARP_H_

#include <sys/prctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/types.h>
#include <stdint.h>
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

#include "CaptureMember.h"
#include "ARPFilterMember.h"

namespace kinow {

class CaptureARP : public CaptureMember {
public:
	CaptureARP(const char* name)
		: CaptureMember(name) { }
	virtual ~CaptureARP() { }
};

} /* namespace kinow */

#endif /* CAPTURE_CAPTUREARP_H_ */
