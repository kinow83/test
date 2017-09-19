/*
 * NetSockFactory.cpp
 *
 *  Created on: 2016. 8. 12.
 *      Author: root
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "NetSockFactory.h"

namespace kinow {

static NetSock*
NetSockFactory::newServer(NET_SOCK_L2_T l2, NET_SOCK_L3_T l3, uint16_t port, int backlog) {
	int l2_type;
	int l3_type;
	int sock;
	struct sockaddr_in  sin4;
	struct sockaddr_in6 sin6;
	struct sockaddr* pSockaddr;
	size_t nSockaddrSize;

	memset(&sin4, 0, sizeof(sin4));
	memset(&sin6, 0, sizeof(sin6));

	switch (l2) {
	// IPv4
	case NET_SCOK_IPV4:
		l2_type = AF_INET;

		sin4.sin_family = AF_INET;
		sin4.sin_addr = htonl(INADDR_ANY);
		sin4.sin_port = htons(port);

		pSockaddr = &sin4;
		nSockaddrSize = sizeof(sin4);
		break;
	// IPv6
	case NET_SCOK_IPV6:
		l2_type = AF_INET6;

		sin6.sin6_family = AF_INET6;
		sin6.sin6_addr = in6addr_any;
		sin6.sin6_port = htons(port);

		pSockaddr = &sin6;
		nSockaddrSize = sizeof(sin6);
		break;
	default:
		return NULL;
	}

	switch (l3) {
	// TCP
	case NET_SCOK_TCP: l3_type = SOCK_STREAM; break;
	// UDP
	case NET_SCOK_UDP: l3_type = SOCK_DGRAM; break;
	default: return NULL;
	}

	sock = socket(l2_type, l3_type, 0);
	if (sock < 0) {
		return NULL;
	}

	if (bind(sock, pSockaddr, nSockaddrSize) == -1) {
		close(sock);
		return NULL;
	}

	if (listen(sock, backlog) == -1) {
		close(sock);
		return NULL;
	}

	return new NetSock(sock);
}

}
static NetSock*
NetSockFactory::newClient(NET_SOCK_L2_T l2, NET_SOCK_L3_T l3, uint16_t port, const char* to) {

}

} /* namespace kinow */
