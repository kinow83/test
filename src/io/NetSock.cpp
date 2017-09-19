/*
 * NetSock.cpp
 *
 *  Created on: 2016. 8. 12.
 *      Author: root
 */

#include <errno.h>
#include <NetSock.h>

namespace kinow {

NetSock::NetSock(int sock) : m_sock(sock) {
}

NetSock::~NetSock() {
	closeSock();
}

int NetSock::getSock() {
	return m_sock;
}
void NetSock::closeSock() {
	if (m_sock != -1) {
		close(m_sock);
		m_sock = -1;
	}
}

bool NetSock::option(int level, int opttype, void* optval, size_t optlen) {
	if (m_sock < 0) {
		return false;
	}
	if (setsockopt(m_sock, level, opttype, optval, optlen)) {
		failure("failed to setsockopt() - %s", strerror(errno));
		return false;
	}
	return true;
}


} /* namespace kinow */
