/*
 * WlanFilterMember.h
 *
 *  Created on: 2016. 9. 26.
 *      Author: root
 */

#ifndef CAPTURE_WLANFILTERMEMBER_H_
#define CAPTURE_WLANFILTERMEMBER_H_

#include <string.h>
#include <sys/types.h>
#include <linux/if_packet.h>

#include "IFilterMember.h"
#include "Error.h"

namespace kinow {

class WlanFilterMember : public IFilterMember {
public:
	WlanFilterMember(const char* name, const char *ifname, const char *filterExpression)
		: IFilterMember(name, ifname, filterExpression) { }
	virtual ~WlanFilterMember() { }

public:
	void filterHandler(uint8_t* buf, size_t len);

private:
	bool parsingWirelessRadiotap(uint8_t* buf, size_t len, IFilterMember *fm);
	bool parsingEthernet(uint8_t* buf, size_t len);

};

} /* namespace kinow */

#endif /* CAPTURE_WLANFILTERMEMBER_H_ */
