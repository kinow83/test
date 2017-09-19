/*
 * test_http.c
 *
 *  Created on: 2016. 11. 6.
 *      Author: root
 */


#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <event.h>
#include <evhttp.h>

void generic_request_handler(struct evhttp_request *req, void *arg)
{
	printf("request on client %s:%d (uri:%s)\n", req->remote_host, req->remote_port, req->uri);
	struct evbuffer *return_buffer = evbuffer_new();

	evbuffer_add_printf(return_buffer, "Thanks for visit my website\n");
	// Send an HTML reply to the client.
	evhttp_send_reply(req, HTTP_OK, "Client", return_buffer);
	evbuffer_free(return_buffer);
	return;
}

int main(int argc, char **argv)
{
	short http_port = 8080;
	char *http_addr = "localhost";
	struct evhttp *http_server = NULL;

	event_init();
	http_server = evhttp_start(http_addr, http_port);
	evhttp_set_gencb(http_server, generic_request_handler, NULL);

	printf("Server started on port %d\n", http_port);

	event_dispatch();

	return 0;

}


