/*
 * IFilterMember.cpp
 *
 *  Created on: 2016. 9. 9.
 *      Author: root
 */

#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <stdint.h>
#include <linux/filter.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "IFilterMember.h"
#include "ieee80211_prism.h"
#include "ieee80211_radiotap.h"
#include "ieee80211_frame.h"
#include "WlanUtils.h"


namespace kinow {

IFilterMember::IFilterMember(const char* name, const char *ifname, const char *filterExpression) {
	m_name = NULL;
	m_ifname = NULL;
	m_filterExpression = NULL;
	m_sock = -1;

	if (name) m_name = strdup(name);
	if (ifname) m_ifname = strdup(ifname);
	if (filterExpression) m_filterExpression = strdup(filterExpression);
}

IFilterMember::~IFilterMember() {
	if (m_name) free(m_name);
	if (m_ifname) free(m_ifname);
	if (m_filterExpression) free(m_filterExpression);
	if (m_sock != -1) close(m_sock);
}

bool IFilterMember::initialize() {
	m_sock = createFilterSocket();
	if (m_sock == -1) {
		return false;
	}
	return true;
}

int IFilterMember::createFilterSocket() {
	int sock = -1;
	bool ok = false;
	struct sock_fprog bpf;
	struct ifreq ifopts;
	int sockopt = 1;
	struct sock_filter_st *sf = NULL;

	// RAW socket and ALL ethernet
	sock = WlanUtils::createSocket(SOCK_RAW, ETH_P_ALL);
	if (sock == -1) {
		failure("FilterMember[%s]: Failed to sock(): %s", m_name, strerror(errno));
		goto out;
	}
	// BFP filter format
	if (m_filterExpression) {
		sf = WlanUtils::socketFilterFactory(m_ifname, m_filterExpression);
		if (sf == NULL) {
			failure("FilterMember[%s]: Failed to get filter code: %s", m_name, m_filterExpression);
			goto out;
		}

		WlanUtils::socketFilterDebug(sf);

		bpf.len = sf->count;
		bpf.filter = sf->filters;
		if(!WlanUtils::setSockAttachFilter(sock, &bpf)) {
			failure("FilterMember[%s]: Failed to setsockopt(SO_ATTACH_FILTER): %s", m_name, strerror(errno));
			goto out;
		}
	}

	if (!WlanUtils::setSockOptAddressReuse(sock)) {
		failure("FilterMember[%s]: Failed to setsockopt(SO_REUSEADDR): %s", m_name, strerror(errno));
		goto out;
	}

	// bind socket to NIC
	if (m_ifname) {
		if (!WlanUtils::setPromiscMode(sock, m_ifname)) {
			failure("FilterMember[%s]: Failed to set promisc mode: %s", m_name, strerror(errno));
			goto out;
		}

		if (!WlanUtils::setSockOptBindDevice(sock, m_ifname))	{
			failure("FilterMember[%s]: Failed to setsockopt(SO_BINDTODEVICE): %s", m_name, strerror(errno));
			goto out;
		}

		if (!WlanUtils::setSockBind(sock, ETH_P_ALL, m_ifname)) {
			failure("FilterMember[%s]: Failed to bind(): %s", m_name, strerror(errno));
			goto out;
		}
	}

	ok = true;

out:
	if (sf) WlanUtils::socketFilterFree(sf);
	if (ok == false) {
		if (sock != -1) {
			close(sock);
			sock = -1;
		}
	}
	return sock;
}

const char* IFilterMember::toString() {
	if (!m_desc[0]) {
		snprintf(m_desc, sizeof(m_desc),
				"FilterMember[name=%s, sock=%d, filter=%s, ifname=%s]",
				m_name, m_sock, m_filterExpression, m_ifname);
	}
	return m_desc;
}

} /* namespace kinow */
