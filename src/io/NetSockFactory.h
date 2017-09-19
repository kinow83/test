/*
 * NetSockFactory.h
 *
 *  Created on: 2016. 8. 12.
 *      Author: root
 */

#ifndef NETSOCKFACTORY_H_
#define NETSOCKFACTORY_H_

#include <NetSock.h>

namespace kinow {

typedef enum NET_SOCK_L2_TYPE {
	NET_SCOK_IPV4,
	NET_SCOK_IPV6,
} NET_SOCK_L2_T;

typedef enum NET_SOCK_L3_TYPE {
	NET_SCOK_TCP,
	NET_SCOK_UDP,
} NET_SOCK_L3_T;

class NetSockFactory {
public:
	static NetSock* newServer(NET_SOCK_L2_T l2,
			NET_SOCK_L3_T l3, uint16_t port, int backlog);
	static NetSock* newClient(NET_SOCK_L2_T l2,
			NET_SOCK_L3_T l3, uint16_t port, const char* to);
private:

private:
	NetSockFactory() {}
	virtual ~NetSockFactory() {}

};

} /* namespace kinow */

#endif /* NETSOCKFACTORY_H_ */
