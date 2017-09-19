/*
 * test_libevent.cpp
 *
 *  Created on: 2016. 11. 6.
 *      Author: root
 */


#include <event.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>


#define SERVICE_PORT 8080
int debug = 0;

struct client {
	int fd;
	struct bufferevent *buf_ev;
};

int set_non_block(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
}

void buf_read_callback2(struct bufferevent *incomming, void *arg)
{
	printf("call buf_read_callback2\n");
}

void buf_read_callback(struct bufferevent *incomming, void *arg)
{
	struct evbuffer *ev_return;
	char *req = NULL;
	struct client *client = (struct client*)arg;
	size_t input_length;
	int echo_enable = 1;

	printf("call buf_read_callback #sock:%d\n", client->fd);

	if (echo_enable) {
//		req = evbuffer_readline(incomming->input);
		req = evbuffer_readln(incomming->input, &input_length, EVBUFFER_EOL_ANY);
		if (req == NULL) {
			return;
		}
		printf("input data length = %ld\n", input_length);

	}
	// Allocate storage for a new evbuffer.
	ev_return = evbuffer_new();

	if (echo_enable) {
		// Append a formatted string to the end of an evbuffer.
		evbuffer_add_printf(ev_return, "You said %s\n", req);
	}

	// Write data from an evbuffer to a bufferevent buffer.	The evbuffer is
	// being drained as a result.
	printf("\t before call bufferevent_write_buffer <input:%p, output:%p>\n",
			incomming->input, incomming->output);
	bufferevent_write_buffer(incomming, ev_return);
	printf("\t after  call bufferevent_write_buffer <input:%p, output:%p>\n",
			incomming->input, incomming->output);

	evbuffer_free(ev_return);
	free(req);
}

void debug_evbuffer(struct evbuffer *evf) {

}

#if 0
void bufferevent_read_and_printf(struct bufferevent *bev) {
	char readbuf[1500];
	int rbytes = bufferevent_read(bev, readbuf, sizeof(readbuf));

	if (rbytes > 0) {
		readbuf[rbytes-1] = 0;
		printf("read data: %s (%d)\n", readbuf, rbytes);
	} else {
		printf("read data: empty data\n");
	}
}
#endif

void buf_write_callback(struct bufferevent *outgoing, void *arg)
{
	printf("call buf_write_callback\n");
	struct evbuffer *ev_write;

#if 0
	ev_write = evbuffer_new();
	evbuffer_add_printf(ev_write, "[kaka]");

	bufferevent_write_buffer(outgoing, ev_write);

	evbuffer_free(ev_write);
#endif

#if 0
	struct evbuffer *incomming_buffer;

	incomming_buffer = bufferevent_get_input(outgoing);
	if (incomming_buffer == NULL) {
		printf("error: failed to get input bufferevent\n");
		return;
	}
#endif

	size_t input_length = 100, output_length = 100;
	char *input_data, *output_data;

	input_data = evbuffer_readln(outgoing->input, &input_length, EVBUFFER_EOL_ANY);
	if (input_data) {
		printf("buf_write_callback: input = %s\n", input_data);
	} else {
		printf("buf_write_callback: input is NULL (%ld), <input=%p, output=%p>\n",
				input_length, outgoing->input, outgoing->output);
	}

	output_data = evbuffer_readln(outgoing->output, &output_length, EVBUFFER_EOL_ANY);
	if (output_data) {
		printf("buf_write_callback: output = %s\n", output_data);
	} else {
		printf("buf_write_callback: output is NULL (%ld) <input=%p, output=%p>\n",
				input_length, outgoing->input, outgoing->output);
	}
}

void buf_error_callback(struct bufferevent *bev, short what, void *arg)
{
	printf("call buf_error_callback\n");

	struct client *client = (struct client*)arg;
	bufferevent_free(client->buf_ev);
	close(client->fd);
	free(client);
}

void accept_callback(int fd, short ev, void *arg)
{
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	struct client *client;

	printf("call accept_callback\n");

	client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd < 0) {
		printf("warn: client: accept() failed\n");
		return;
	}

	set_non_block(client_fd);

	client = (struct client*)calloc(1, sizeof(*client));
	if (client == NULL) {
		printf("error: malloc failed\n");
	}
	client->fd = client_fd;
	client->buf_ev = bufferevent_new(client_fd,
			buf_read_callback,
			buf_write_callback,
			buf_error_callback,
			client);

	// Enable a bufferevent.
	// @param event any combination of EV_READ | EV_WRITE.
	bufferevent_enable(client->buf_ev, EV_READ);
}

int main(int argc, char **argv)
{
	int socket_listen;
	struct sockaddr_in addr_listen;
	struct event accept_event;
	int reuse = 1;


	// Initialize the event API.
	event_init();

	socket_listen = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_listen < 0) {
		printf("error: failed to create listen socket\n");
		return -1;
	}

	memset(&addr_listen, 0, sizeof(addr_listen));

	addr_listen.sin_family = AF_INET;
	addr_listen.sin_addr.s_addr = INADDR_ANY;
	addr_listen.sin_port = htons(SERVICE_PORT);


	setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));


	if (bind(socket_listen, (struct sockaddr *)&addr_listen,
			sizeof(addr_listen)) < 0)
	{
		printf("error: failed to bind\n");
		return -1;
	}

	if((listen(socket_listen, 5)) < 0)
	{
		printf("error: failed to listen to socket\n");
		return -1;
	}

	event_set(&accept_event,
			socket_listen,
			EV_READ | EV_PERSIST,
			accept_callback,
			NULL);

	event_add(&accept_event, NULL);

	printf("call before event_dispatch\n");
	event_dispatch();
	printf("call after  event_dispatch\n");

	close(socket_listen);

	return 0;
}







