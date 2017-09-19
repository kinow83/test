/*
 * read_timeout.c
 *
 *  Created on: 2017. 9. 10.
 *      Author: root
 */
#include <event2/event.h>

void cb_func(evutil_socket_t fd, short what, void *arg)
{

}

void read_persist_loop(evutil_socket_t fd)
{
	struct evnet *ev;
	struct timeval tv = {5, 0};

	ev = event_new(base)
}

int main()
{
	struct event_base *base;

	base = event_base_new();
	event_base_priority_init(base, 2);



}



