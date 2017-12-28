#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include "epoller.h"

int epoller_wait(Epoller *e, void *data)
{
	struct epoll_event ev, evs;
	int efd;
	int res;

	if (e->read_handler == NULL) {
		return -1;
	}

	efd = epoll_create(1);
	if (efd < 0) {
		return -1;
	}
	ev.events = EPOLLIN;
	ev.data.fd = e->sock;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, e->sock, &ev) < 0) {
		close(efd);
		return -1;
	}

	while (1) {
		res = epoll_wait(efd, &ev, 1, e->timeout);
		if (res < 0) {
			if (errno == EINTR) continue;
			close(efd);
			return -1;
		}
		else if (res == 0) {
			if (e->timeout_handler && e->timeout_handler(e, data) == -1) {
				close(efd);
				return -1;
			}
		}
		if (e->read_handler(e, data) == -1) {
			close(efd);
			return -1;
		}
	}
	close(efd);
	return 0;
}

