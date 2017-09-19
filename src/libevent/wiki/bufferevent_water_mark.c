/*
 * bufferevent_water_mark.c
 *
 *  Created on: 2017. 9. 16.
 *      Author: root
 */


#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "libsock.h"

void write_callback(struct bufferevent *bev, void *ctx)
{
	/*
	printf("call write_callback\n");
	struct evbuffer *output;

	output = bufferevent_get_output(bev);
//	evbuffer_add_printf(output, "[kaka]\n");
	printf("end write_callback\n");
//	sleep(2);
 */
}

void read_callback(struct bufferevent *bev, void *ctx)
{
	printf("call read_callback\n");
	struct event_base *base = ctx;
	struct evbuffer *input, *output;
	char buf[1024];
	int n;


#if 0
	input = bufferevent_get_input(bev);
	int len = evbuffer_get_length(input);
	if (len) {
		n = evbuffer_remove(input, buf, sizeof(buf));
		//printf("recv %d/%d\n", len, n);
		if (n >= 2) {
			if (buf[n-2] == '\r') {
				buf[n-2] = 0;
				n = n - 2;
			}
			else if (buf[n-1] == '\n') {
				buf[n-1] = 0;
				n--;
			}
			else {
				buf[n] = 0;
			}

			printf("%s(%d/%d)\n", buf, n, len);
		}

		/*
		evbuffer_drain(input, len);
		printf("Drained %lu bytes from %s\n", (unsigned long) len);
		*/
	}
#else
	size_t len = bufferevent_read(bev, buf, sizeof(buf));
	if (len > 0) {
		buf[len] = 0;
		printf("%s\n", buf);
	}
	const char *msg = "kakakakakaka";
	bufferevent_write(bev, msg, strlen(msg));

	output = bufferevent_get_output(bev);
	evbuffer_add_printf(output, "[kaka]\n");
#endif
}

void print_event_status(short events)
{
	if (events & BEV_EVENT_CONNECTED) {
		printf("\t BEV_EVENT_CONNECTED \n");
	}
	if (events & BEV_EVENT_TIMEOUT) {
		printf("\t BEV_EVENT_TIMEOUT \n");
	}
	if (events & BEV_EVENT_ERROR) {
		printf("\t BEV_EVENT_ERROR \n");
	}
	if (events & BEV_EVENT_EOF) {
		printf("\t BEV_EVENT_EOF \n");
	}
	if (events & BEV_EVENT_WRITING) {
		printf("\t BEV_EVENT_WRITING \n");
	}
	if (events & BEV_EVENT_READING) {
		printf("\t BEV_EVENT_READING \n");
	}
}

void event_callback(struct bufferevent *bev, short events, void *ctx)
{
	printf("call event_callback\n");
	print_event_status(events);
	if(events & BEV_EVENT_CONNECTED) {
		printf("Connect okay.\n");
	}
	else if(events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
		struct event_base *base = ctx;
		if (events & BEV_EVENT_ERROR) {
			int err = bufferevent_socket_get_dns_error(bev);
			if (err)
				printf("DNS error: %s\n", evutil_gai_strerror(err));
		}
		else if (events & BEV_EVENT_EOF) {
			printf("eof\n");
		}
		printf("Closing\n");
		bufferevent_free(bev);
		event_base_loopexit(base, NULL);
	}
}

struct bufferevent *setup_bufferevent(struct event_base *base, int client_fd)
{
	struct bufferevent *bev;
	int listen_fd;
	struct timeval rd_timeout = {
			.tv_sec = 20,
			.tv_usec = 0,
	};

	bev = bufferevent_socket_new(
			base, client_fd,
			BEV_OPT_CLOSE_ON_FREE);
	//bufferevent_setwatermark(bev, EV_READ, 10, 0);
	bufferevent_setcb(bev,
			read_callback,
			write_callback,
			event_callback,
			base);
	bufferevent_set_timeouts(bev, &rd_timeout, NULL);
	bufferevent_enable(bev, EV_READ | EV_WRITE);
	return bev;
}


int main()
{
	struct bufferevent *bev;
	struct event_base *base;
	int listen_fd;
	int client_fd;
	socklen_t caddr_len;
	struct sockaddr_in caddr;

	base = event_base_new();

	listen_fd = socket_new(1234);
	listen(listen_fd, 5);

	client_fd = accept(listen_fd, (struct sockaddr *)&caddr, &caddr_len);
	printf("accept\n");

	bev = setup_bufferevent(base, client_fd);

	event_base_dispatch(base);

	close(listen_fd);
	bufferevent_free(bev);
}
