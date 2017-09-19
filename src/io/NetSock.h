/*
 * NetSock.h
 *
 *  Created on: 2016. 8. 12.
 *      Author: root
 */

#ifndef NETSOCK_H_
#define NETSOCK_H_

#include "Error.h"

namespace kinow {

class NetSock: public Error {
public:
	NetSock(int sock);
	virtual ~NetSock();

public:
	int getSock();
	void closeSock();
	bool option(int level, int opttype, void* optval, size_t optlen);

private:
	int m_sock;
};

} /* namespace kinow */

#endif /* NETSOCK_H_ */
