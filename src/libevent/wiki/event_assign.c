#include <event2/event.h>
#include <event2/event_struct.h>
#include <stdlib.h>
#include <unistd.h>
#include "libsock.h"
#include <string.h>

struct event_pair {
	evutil_socket_t fd;
	struct event rd_event;
	struct event wr_event;
};

void read_cb(evutil_socket_t fd, short what, void *arg)
{
	printf("call read_cb\n");
	char buf[1024];
	int bytes;

	bytes = read(fd, buf, sizeof(buf));
	buf[bytes] = 0;
	printf("%s\n", buf);
	write(fd, buf, bytes);
}

void write_cb(evutil_socket_t fd, short what, void *arg)
{
	printf("call write_cb\n");
	const char *msg = "hi";
	size_t msg_len = strlen(msg);
	int bytes = write(fd, msg, msg_len);
	printf("write %d\n", bytes);
}

struct event_pair *event_pair_new(struct event_base *base, evutil_socket_t fd)
{
	struct event_pair *p = malloc(sizeof(struct event_pair));
	if (!p) return NULL;

	p->fd = fd;
	if (event_assign(&p->rd_event, base, fd, EV_READ |EV_PERSIST, read_cb,  (void *)p)) {
		printf("read_cb add fail\n");
	}
	printf("add read_cb\n");
	if (event_assign(&p->wr_event, base, fd, EV_WRITE|EV_PERSIST, write_cb, (void *)p)) {
		printf("write_cb add fail\n");
	}
	printf("add write_cb\n");
	printf("pair = %p\n", p);

	event_add(&p->rd_event, NULL);
//	event_add(&p->wr_event, NULL);
	return p;
}

int main()
{
	struct event_base *base;
	struct event_pair *pair;
	int listen_fd;
	socklen_t caddr_len;
	int client_fd;
	struct sockaddr_in caddr;

	listen_fd = socket_new(1234);
	if (listen_fd < 0) {
		perror("socket_new()");
		exit(1);
	}
	base = event_base_new();
	if (base < 0) {
		printf("base is NULL\n");
		exit(1);
	}

	if (listen(listen_fd, 5) < 0) {
		perror("listen()");
		exit(1);
	}
	printf("listen = %d\n", listen_fd);

	client_fd = accept(listen_fd, (struct sockaddr *)&caddr, &caddr_len);


#if 1
	pair = event_pair_new(base, client_fd); 
	if (!pair) {
		perror("event_pair_new()");
		exit(1);
	}
#else
	struct event *r = event_new(base, client_fd, EV_READ |EV_PERSIST, read_cb,  base);
	event_add(r, NULL);
	//struct event *w = event_new(base, client_fd, EV_WRITE|EV_PERSIST, write_cb, base);
	//event_add(w, NULL);
#endif
	printf("client=%d assigned\n", client_fd);

	printf("before dispatch\n");
	event_base_dispatch(base);
//	event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
//	event_base_loop(base, EVLOOP_ONCE);
	printf("after dispatch\n");

	sleep(2);
	close(client_fd);
	close(listen_fd);
	//free(pair);
	event_base_free(base);
}
