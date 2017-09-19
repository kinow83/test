/*
 * test_proxy.c
 *
 *  Created on: 2017. 9. 17.
 *      Author: root
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <event.h>
#include <event2/bufferevent_ssl.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <pthread.h>
#include <signal.h>

#include "libsock.h"
#include "work_queue.h"

struct conn_info {
	int sockfd;
	struct sockaddr_in addr;
};

struct proxy_info {
	struct conn_info client;
	struct conn_info server;
	struct event_base *evbase;
};

static int backend_port = 143;
static int listen_port = 10143;

static struct event_base *evbase_accept;
static struct worker_queue worker_queue;


void client_to_server(int fd, short waht, void *arg)
{
	struct proxy_info *proxy_info = (struct proxy_info *)arg;
	char buf[1024];
	int rbytes, wbytes;
	int server_fd = proxy_info->server.sockfd;

	rbytes = wbytes = 0;
	while ((rbytes = read(fd, buf, sizeof(buf))) > 0) {
		printf("client_to_server read=%d\n", rbytes);

		wbytes = write(server_fd, buf, rbytes);
		if (rbytes != wbytes) {
			printf("!!!! abort proxy rbytes(%d) != wbytes(%d) !!!!\n", rbytes, wbytes);
		}

		buf[wbytes] = 0;
		printf("c->s: %s\n", buf);
	}
	// close session.
	if (rbytes <= 0) {
		if (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN) {
			/*
			[read]
			BLOCK     : read 버퍼가 비어있을때 block
			NON-BLOCK : read 버퍼가 비어있을때 -1 return, errno==EWOULDBLOCK/EAGAIN

			[write]
			BLOCK     : write 버퍼가 꽉 차있을때 block
			NON-BLOCK : write 버퍼가 꽉 차있을때 -1 return, errno==EWOULDBLOCK/EAGAIN

			[accept]
			BLOCK     : backlog( 현재의 connection 요청 큐 )가 비어있을때 block
			NON-BLOCK : backlog( 현재의 connection 요청 큐 )가 비어있을때 -1 return, errno==EWOULDBLOCK/EAGAIN

			[connect]
			BLOCK     : connection이 완전히 이루어질때까지 block
			NON-BLOCK : connection이 완전히 이루어지 않더라도 곧바로 return. 나중에 getsockopt로 connection이 완전히 이루어졌는지 확인가능.
			*/
		} else {
			printf("client_to_server: %s (%d)\n", strerror(errno), errno);
			event_base_loopbreak(proxy_info->evbase);
		}
	}
	if (wbytes <= 0) {
		printf("%s (%d)\n", strerror(errno), errno);
		event_base_loopbreak(proxy_info->evbase);
	}
}

void server_to_client(int fd, short waht, void *arg)
{
	struct proxy_info *proxy_info = (struct proxy_info *)arg;
	char buf[1024];
	int rbytes, wbytes;
	int client_fd = proxy_info->client.sockfd;

	rbytes = wbytes = 0;
	while ((rbytes = read(fd, buf, sizeof(buf))) > 0) {
		printf("server_to_client read=%d\n", rbytes);

		wbytes = write(client_fd, buf, rbytes);
		if (rbytes != wbytes) {
			printf("!!!! abort proxy rbytes(%d) != wbytes(%d) !!!!\n", rbytes, wbytes);
		}

		buf[wbytes] = 0;
		printf("s->c: %s\n", buf);
	}
	// close session.
	if (rbytes <= 0) {
		if (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN) {
		} else {
			printf("server_to_client: %s (%d)\n", strerror(errno), errno);
			event_base_loopbreak(proxy_info->evbase);
		}
	}
	if (wbytes <= 0) {
		printf("%s (%d)\n", strerror(errno), errno);
		event_base_loopbreak(proxy_info->evbase);
	}
}

static void proxy_job_run(struct job *job)
{
	struct proxy_info *proxy_info = (struct proxy_info *)job->ctx;

	event_base_dispatch(proxy_info->evbase);

	printf("close client:%d (%d) <- -> server:%d (%d)\n",
			proxy_info->client.sockfd,
			ntohs(proxy_info->client.addr.sin_port),
			proxy_info->server.sockfd,
			ntohs(proxy_info->server.addr.sin_port));

	if (proxy_info->client.sockfd > 0)
		close(proxy_info->client.sockfd);
	if (proxy_info->server.sockfd > 0)
		close(proxy_info->server.sockfd);

	event_base_free(proxy_info->evbase);

	free(proxy_info);
	free(job);
}

void on_accept(int fd, short waht, void *arg)
{
	int i;
	int client_fd, server_fd;
	struct sockaddr_in caddr, saddr;
	static socklen_t caddrlen = sizeof(caddr);
	struct worker_queue *worker_queue = (struct worker_queue *)arg;
	struct proxy_info *proxy_info;
	struct job *job;

	client_fd = accept(fd, (struct sockaddr *)&caddr, &caddrlen);
	if (client_fd < 0) {
		perror("accept()");
		return;
	}
	evutil_make_socket_nonblocking(client_fd);

	saddr.sin_family = AF_INET,
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1"),
	saddr.sin_port = htons(backend_port),

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(server_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		perror("connect()");
		close(client_fd);
		return;
	}
	evutil_make_socket_nonblocking(server_fd);

	proxy_info = (struct proxy_info *)malloc(sizeof(struct proxy_info));
	proxy_info->client.sockfd = client_fd;
	memcpy(&proxy_info->client.addr, &caddr, sizeof(caddr));
	proxy_info->server.sockfd = server_fd;
	memcpy(&proxy_info->server.addr, &saddr, sizeof(saddr));

	proxy_info->evbase = event_base_new();
	if (!proxy_info->evbase) {
		perror("event_base_new()");
		close(client_fd);
		close(server_fd);
		free(proxy_info);
		return;
	}

	worker_queue_init(worker_queue, 5);

	struct event *ev_ctos = event_new(
			proxy_info->evbase,
			client_fd,
			EV_READ|EV_PERSIST,
			client_to_server,
			proxy_info);

	struct event *ev_stoc = event_new(
			proxy_info->evbase,
			server_fd,
			EV_READ|EV_PERSIST,
			server_to_client,
			proxy_info);

	event_add(ev_ctos, NULL);
	event_add(ev_stoc, NULL);

	job = (struct job *)malloc(sizeof(struct job));
	job->run = proxy_job_run;
	job->ctx = proxy_info;

	printf("new client:%d (%d) <- -> server:%d (%d)\n",
			proxy_info->client.sockfd,
			ntohs(proxy_info->client.addr.sin_port),
			proxy_info->server.sockfd,
			ntohs(proxy_info->server.addr.sin_port));

	worker_queue_add_job(worker_queue, job);
}

static void kill_proxy(void)
{
	if (event_base_loopexit(evbase_accept, NULL)) {
		perror("event_base_loopexit()");
	}
	worker_queue_shutdown(&worker_queue);
}

static void sig_handler(int signo)
{
	kill_proxy();
}

static int run_proxy()
{
	int listen_fd;
	struct sockaddr_in listen_addr;
	struct event ev_accept;

	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	evbase_accept = event_base_new();

	listen_fd = socket_new(listen_port);
	if (listen_fd < 0) {
		perror("socket()");
		goto clean;
	}
	evutil_make_socket_nonblocking(listen_fd);

	listen(listen_fd, 5);

	event_assign(
			&ev_accept,
			evbase_accept,
			listen_fd,
			EV_READ|EV_PERSIST,
			on_accept,
			(void *)&worker_queue);

	event_add(&ev_accept, NULL);

	printf("server running.\n");

	event_base_dispatch(evbase_accept);

clean:
	event_base_free(evbase_accept);
	evbase_accept = NULL;
	if (listen_fd > 0) close(listen_fd);

	printf("server shutdown.\n");
	return 0;
}

int main()
{
	printf("listen port : %d\n", listen_port);
	printf("backend port: %d\n", backend_port);
	return run_proxy();
}
