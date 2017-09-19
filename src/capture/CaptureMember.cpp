/*
 * CaptureMember.cpp
 *
 *  Created on: 2016. 9. 9.
 *      Author: root
 */

#include <sys/socket.h>
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
#include <sys/types.h>
#include <sys/epoll.h>
#include <stdlib.h>

#include "WlanFilterMember.h"
#include "CaptureMember.h"
#include "WlanUtils.h"

namespace kinow {


bool addEpollFD(int epollFd, int addFd) {
	struct epoll_event ev;
	int ret;

	ev.events = EPOLLIN;
	ev.data.fd = addFd;
	return !epoll_ctl(epollFd, EPOLL_CTL_ADD, addFd, &ev);
}

bool delEpollFD(int epollFd, int addFd) {
	struct epoll_event ev;
	return !epoll_ctl(epollFd, EPOLL_CTL_DEL, addFd, &ev);
}

CaptureMember::CaptureMember(const char* name) {
	m_name = NULL;

	if (name) m_name = strdup(name);
}

CaptureMember::~CaptureMember() {
	int i;
	IFilterMember *fm;

	for (i=0; i<m_filterMembers.size(); i++) {
		fm = m_filterMembers[i];
		delete fm;
	}
	if (m_name) free(m_name);
}

void CaptureMember::addFilterMember(IFilterMember *fm) {
	if (!fm) return;

	m_filterMembers.push_back(fm);
	if (!fm->initialize()) {
		fprintf(stderr, "CaptureMember[%s]: Failed to FilterMember - %s\n", m_name, fm->cause());
	} else {
		printf("CaptureMember[%s]: %s\n", m_name, fm->toString());
	}
}

bool CaptureMember::initialize() {
	return true;
}

bool CaptureMember::CaptureMember::capture() {
	int epollFd, recvFd;
	int i;
	bool ok = false;
	int event_cnt;
	static int timeout = 100; // 100ms
	int max_event_num = 0;
	struct epoll_event *evlist = NULL;
	IFilterMember **fmArray = NULL;
	IFilterMember *fm = NULL;
	int maxSockNum = 0;
	int bytes;
	static int mtu = 1500;
	u_char buf[1500];
	struct sockaddr sockaddr;
	int sockaddrSize = sizeof(sockaddr);

	epollFd = epoll_create(1);
	if (epollFd == -1) {
		failure("CaptureMember[%s]: Failed to epoll_create(): %s", m_name, strerror(errno));
		goto out;
	}

	maxSockNum = getFilterMemberMaxFD() + 1;
	fmArray = (IFilterMember**)malloc(sizeof(IFilterMember*) * maxSockNum);

	for (i=0; i<m_filterMembers.size(); i++) {
		fm = m_filterMembers[i];
		if (addEpollFD(epollFd, fm->getSock()) == false) {
			failure("CaptureMember[%s]: FilterMember[%s] Failed to epool add: %s", m_name, fm->getName(), strerror(errno));
			goto out;
		}
		// Fast search FilterMember using socket number !!
		fmArray[fm->getSock()] = fm;
		max_event_num++;
	}

	printf("CaptureMember[%s]: max event number is [%d]\n", m_name, max_event_num);
	evlist = (struct epoll_event*)malloc(sizeof(struct epoll_event) * max_event_num);
	memset(evlist, 0, sizeof(struct epoll_event) * max_event_num);

	ok = true;

	while (true) {
		event_cnt = epoll_wait(epollFd, evlist, max_event_num, timeout);
		if (event_cnt == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				failure("CaptureMember[%s]: Failed to epoll_wait: %s", m_name, strerror(errno));
				continue;
			}
		}
		for (i=0; i<event_cnt; i++) {
			recvFd = evlist[i].data.fd;

			if (maxSockNum < recvFd) {
				failure("CaptureMember[%s]: What the hell?? event sock is [%d] but max socket is [%d]", m_name, recvFd, maxSockNum);
			} else {
				fm = fmArray[recvFd];
//				printf("CaptureMember[%s]: raise event %s\n", m_name, fm->getName());

				bytes = recvfrom(recvFd, buf, sizeof(buf), 0,
						(struct sockaddr*)&sockaddr, (socklen_t*)&sockaddrSize);
				fm->filterHandler(buf, bytes);
			}
		}
	}
out:
	if (epollFd != -1) close(epollFd);
	if (evlist) free(evlist);
	if (fmArray) free(fmArray);
	return ok;
}

int CaptureMember::getFilterMemberMaxFD() {
	int i;
	int max = 0;
	IFilterMember *fm;

	for (i=0; i<m_filterMembers.size(); i++) {
		fm = m_filterMembers[i];
		if (max < fm->getSock()) {
			max = fm->getSock();
		}
	}
	return max;
}

} /* namespace kinow */

#ifdef CAP_TEST
using namespace kinow;

int main(int argc, char** argv) {
	const char *dev = argv[1];

	CaptureMember *cm = NULL;
	IFilterMember *fm1 = NULL, *fm2 = NULL, *fm3 = NULL;
	IFilterMember *main = NULL;

	cm = new CaptureMember("CM");

	/**
	 * TCPDUMP filter specifics
	 * http://www.tcpdump.org/manpages/pcap-filter.7.txt
	 */

//	main = new WlanFilterMember("beacon", dev, "type mgt subtype beacon");
	fm1 = new WlanFilterMember("12:e3:c7:07:3b:01", dev, "ether src 12:e3:c7:07:3b:01");
	fm2 = new WlanFilterMember("SSH ", dev, "port 22");
	fm3 = new WlanFilterMember("DNS ", dev, "port 53");

	cm->addFilterMember(main);
	cm->addFilterMember(fm1);
	cm->addFilterMember(fm2);
	cm->addFilterMember(fm3);


	printf("DLT: %d\n", WlanUtils::getDLT(dev));

	cm->capture();
}
#endif
