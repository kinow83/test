/*
 * IFilterMember.h
 *
 *  Created on: 2016. 9. 9.
 *      Author: root
 */

#ifndef CAPTURE_IFILTERMEMBER_H_
#define CAPTURE_IFILTERMEMBER_H_

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <linux/if_packet.h>

#include "Error.h"

namespace kinow {

using namespace std;

class IFilterMember : public Error {
public:
	IFilterMember(const char* name, const char *ifname, const char *filterExpression);
	virtual ~IFilterMember();

	int getSock() { return m_sock; }
	const char* getFilterExpression() { return m_filterExpression; }
	const char* getIfname() { return m_ifname; }
	const char* getName() { return m_name; }
	const char* toString();
	bool initialize();

public:
	virtual void filterHandler(uint8_t* buf, size_t len) = 0;

private:
	int createFilterSocket();

private:
	char *m_name;
	char *m_ifname;
	char *m_filterExpression;
	int m_sock;
	char m_desc[128];
};

} /* namespace kinow */

#endif /* CAPTURE_IFILTERMEMBER_H_ */
