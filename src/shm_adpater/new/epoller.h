#ifndef __EPOLLER__
#define __EPOLLER__

struct Epoller;

typedef int (*epoll_handler)(struct Epoller *, void *);

typedef struct Epoller {
	int sock;
	int timeout;
	epoll_handler read_handler;
	epoll_handler timeout_handler;
} Epoller;

int epoller_wait(Epoller *e, void *data);

#endif
