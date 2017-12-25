#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/epoll.h>
#include "queue.h"
#include "sock.h"
#include "pbio_data.h"
#include "xid.h"

/* [svc port map] ************************/
typedef struct svc_portmap_t {
	const char *svc_name;
	uint16_t   svc_port;
	const char *proxy_uds_sofile;
} svc_portmap_t;

static svc_portmap_t svc_portmaps[3] = {
	{"http", 8800, "/tmp/http.sock"},
	{"snmp", 8801, "/tmp/snmp.sock"},
	{"pop3", 8802, "/tmp/pop3.sock"},
};

/* [] ************************/
typedef struct svc_sock_t {
	int lsock;
	int csock;
	struct sockaddr_in caddr;
} svc_sock_t;

/* [pb io link] ************************/
typedef struct pbio_link_t {
	char     svc_name[64];
	int      svc_lsock;
	uint16_t svc_lport;

	int      proxy_lsock;
	int      proxy_csock;
	char     proxy_uds_sofile[128];
} pbio_link_t;

#define NUM_PBIO_NODES 1500
static pbio_link_t pbio_links[NUM_PBIO_NODES];
static pthread_mutex_t pbio_link_lock = PTHREAD_MUTEX_INITIALIZER;


/* [accept sock queue] ************************/
static pthread_mutex_t svc_accept_queue_lock = PTHREAD_MUTEX_INITIALIZER;
static queue_t* svc_accept_queues;

/* [number of listen sock] ************************/
static int num_svc_listens;
static int num_pbio_link_listens;

/* [] ************************/
typedef struct pbio_link_data_t {
	xid_t xid;
	uint16_t index;
} __attribute__((packed)) pbio_link_data_t;

/* [] ************************/
static pbio_data_queue_t *pbio_data_queues;
static pthread_mutex_t pbio_data_queue_lock = PTHREAD_MUTEX_INITIALIZER;

/* [service worker thread] ************************/
void *svc_worker_thread(void *arg)
{
	int noop = 0;
	int proxy_csock;
	size_t nbytes;
	svc_sock_t *svc_sock = NULL;
	pbio_link_data_t link_data;
	pbio_data_t *pbio_data;

retry:
	/* accquire accept sock */
	while (1) {
		pthread_mutex_lock(&svc_accept_queue_lock);
		{
			svc_sock = dequeue(svc_accept_queues);
			if (!svc_sock) {
				pthread_mutex_unlock(&svc_accept_queue_lock);
//				printf("svc worker queue is EMPTY!!!\n");
				if (noop++ > 100) {
					usleep(100);
					noop = 0;
				}
				continue;
			}
			noop = 0;
		}
		pthread_mutex_unlock(&svc_accept_queue_lock);
		break;
	}

	if (gen_xid(&link_data.xid, &svc_sock->caddr)) {
		goto retry;
	}

	/* check proxy client connection */
	pthread_mutex_lock(&pbio_link_lock);
	{
		proxy_csock = pbio_links[svc_sock->lsock].proxy_csock;
	}
	pthread_mutex_unlock(&pbio_link_lock);
	if (proxy_csock <= 0) {
		printf("'%s' proxy client doesn't connected. can't service. closed\n", 
				pbio_links[svc_sock->lsock].svc_name);
		close(svc_sock->csock);
		free(svc_sock);
		goto retry;
	}


	struct epoll_event ev, evs;
	int efd;
	int res;
	efd = epoll_create(1);
	if (efd < 0) {
		goto retry;
	}
	ev.events = EPOLLIN;
	ev.data.fd = svc_sock->csock;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, svc_sock->csock, &ev) < 0) {
		perror("epoll_ctl:ADD");
		goto retry;
	}
	while (1) {
		res = epoll_wait(efd, &ev, 1, 100);
		if (res < 0) {
			if (errno == EINTR) continue;
			break;
		} else if (res != 1) {
			continue;
		}

		pthread_mutex_lock(&pbio_data_queue_lock);
		{
			pbio_data = alloc_pbio_data_queue(pbio_data_queues, &link_data.index);
			printf("alloc: %p, index:%d\n", pbio_data, link_data.index);
		}
		pthread_mutex_unlock(&pbio_data_queue_lock);

		nbytes = read(svc_sock->csock, pbio_data->data, PBIO_DATA_SIZE);
		if (nbytes < 0) {
			if (errno == EINTR) continue;
		}
		if (nbytes <= 0) {
			break;
		}
		pbio_data->datalen = nbytes;

		printf("%s (%ld)\n", pbio_data->data, pbio_data->datalen);
		// send buf to proxy client
		nbytes = write(proxy_csock, &link_data, sizeof(link_data));
	}
	close(svc_sock->csock);
	free(svc_sock);
	goto retry;

	return NULL;
}

void init_svc_accept()
{
	int i;
	int lsock, lport;
	int num_svc_portmaps = sizeof(svc_portmaps) / sizeof(svc_portmap_t);

	num_svc_listens = 0;
	for (i=0; i<num_svc_portmaps; i++) {
		lport = svc_portmaps[i].svc_port;
		lsock = tcp_listen_sock(lport);
		pthread_mutex_lock(&pbio_link_lock);
		{
			pbio_links[lsock].svc_lsock = lsock;
			pbio_links[lsock].svc_lport = lport;
			snprintf(pbio_links[lsock].svc_name,          64, "%s", svc_portmaps[i].svc_name);
			snprintf(pbio_links[lsock].proxy_uds_sofile, 128, "%s", svc_portmaps[i].proxy_uds_sofile);
			printf("%s(%d): listen svc: listen:%d, port:%d\n", svc_portmaps[i].svc_name, lsock, lsock, lport);
		}
		pthread_mutex_unlock(&pbio_link_lock);
		num_svc_listens++;
	}
}

void init_pbio_link_accept()
{
	int i;
	int lsock, lport;
	const char *proxy_uds_sofile;

	num_pbio_link_listens = 0;
	for (i=1; i<NUM_PBIO_NODES; i++) {
		pthread_mutex_lock(&pbio_link_lock);
		{
			lsock = pbio_links[i].svc_lsock;
			if (lsock == i) {
				lsock = uds_listen_sock(pbio_links[lsock].proxy_uds_sofile);
				if (lsock < 0) {
					perror("uds_listen_sock()");
					exit(1);
				}
				pbio_links[i].proxy_lsock = lsock;
				printf("%s(%d): listen pbio: listen:%d, uds:%s\n", 
					pbio_links[i].svc_name, i, lsock, pbio_links[i].proxy_uds_sofile);
			}
		}
		pthread_mutex_unlock(&pbio_link_lock);
		num_pbio_link_listens++;
	}
}

/* [accept for proxy client] ************************/
void *pbio_link_accept_thread(void *arg)
{
	int i, k, res;
	int efd = -1;
	int lsock, csock, svc_lsock;
	int proxy_lsock;
	struct epoll_event ev, *evs = NULL;
	struct sockaddr_un caddr;
	socklen_t caddrlen = sizeof(caddr);

	efd = epoll_create(num_pbio_link_listens);
	if (efd < 0) {
		goto done;
	}
	evs = (struct epoll_event*)malloc(sizeof(struct epoll_event) * num_pbio_link_listens);
	for (i=1; i<NUM_PBIO_NODES; i++) {
		lsock = pbio_links[i].svc_lsock;
		if (lsock == i) {
			proxy_lsock = pbio_links[i].proxy_lsock;
			ev.events = EPOLLIN;
			ev.data.fd = proxy_lsock;
			if (epoll_ctl(efd, EPOLL_CTL_ADD, proxy_lsock, &ev) < 0) {
				perror("epoll_ctl:ADD");
				goto done;
			}
			printf("%s added pbio list accept epoll, sock:%d\n", pbio_links[i].svc_name, proxy_lsock);
		}
	}

	while (1) {
		res = epoll_wait(efd, evs, num_pbio_link_listens, 100);
		if (res < 0) {
			if (errno == EINTR) continue;
			break;
		} else if (res == 0) {
			continue;
		}
		for (i=0; i<res; i++) {
			lsock = evs[i].data.fd;
			csock = accept(lsock, (struct sockaddr*)&caddr, &caddrlen);

			pthread_mutex_lock(&svc_accept_queue_lock);
			{
				for (k=1; k<NUM_PBIO_NODES; k++) {
					svc_lsock = pbio_links[k].svc_lsock;
					proxy_lsock = pbio_links[k].proxy_lsock;
					if ((svc_lsock == k) && (proxy_lsock == lsock)) {
						pbio_links[k].proxy_csock = csock;
						printf("%s pbio link connected...\n", pbio_links[k].svc_name);
						break;
					}
				}
			}
			pthread_mutex_unlock(&svc_accept_queue_lock);
		}
	}
done:
	if (evs > 0) free(evs);
	for (i=1; i<NUM_PBIO_NODES; i++) {
		lsock = pbio_links[i].svc_lsock;
		if (lsock == i) {
			close(lsock);
		}
	}
	if (efd > 0) close(efd);
	return NULL;
}

/* [service accept thread] ************************/
void *svc_accept_thread(void *arg)
{
	int i, res;
	int lsock, csock;
	int efd = -1;
	struct epoll_event ev, *evs = NULL;
	svc_sock_t *svc_sock;
	struct sockaddr_in caddr;
	socklen_t caddrlen = sizeof(caddr);

	efd = epoll_create(num_svc_listens);
	if (efd < 0) {
		goto done;
	}
	evs = (struct epoll_event*)malloc(sizeof(struct epoll_event) * num_svc_listens);
	for (i=1; i<NUM_PBIO_NODES; i++) {
		lsock = pbio_links[i].svc_lsock;
		if (lsock == i) {
			ev.events = EPOLLIN;
			ev.data.fd = lsock;
			if (epoll_ctl(efd, EPOLL_CTL_ADD, lsock, &ev) < 0) {
				perror("epoll_ctl:ADD");
				goto done;
			}
			printf("%s added svc accept epoll, sock:%d\n", pbio_links[i].svc_name, lsock);
		}
	}

	while (1) {
		res = epoll_wait(efd, evs, num_svc_listens, 100);
		if (res < 0) {
			if (errno == EINTR) continue;
			break;
		} else if (res == 0) {
			continue;
		}
		for (i=0; i<res; i++) {
			lsock = evs[i].data.fd;
			csock = accept(lsock, (struct sockaddr*)&caddr, &caddrlen);
			printf("%s connected...\n", pbio_links[lsock].svc_name);

			svc_sock = malloc(sizeof(*svc_sock));
			svc_sock->lsock = lsock;
			svc_sock->csock = csock;
			memcpy(&svc_sock->caddr, &caddr, sizeof(caddr));

			pthread_mutex_lock(&svc_accept_queue_lock);
			{
				if (enqueue(svc_accept_queues, svc_sock) < 0) {
					printf("svc accept queue is FULL!!!\n");
				}
			}
			pthread_mutex_unlock(&svc_accept_queue_lock);
		}
	}
done:
	if (evs > 0) free(evs);
	for (i=1; i<NUM_PBIO_NODES; i++) {
		lsock = pbio_links[i].svc_lsock;
		if (lsock == i) {
			close(lsock);
		}
	}
	if (efd > 0) close(efd);
	return NULL;
}

int main(int argc, char **argv)
{
	const static int num_workers = 2;
	struct sockaddr_in caddr;
	int ssock, csock;
	pthread_t svc_accept_tid;
	pthread_t pbio_accept_tid;
	pthread_t svc_worker_tid[num_workers];
	pthread_t iolink_tid;
	int i;

	pbio_data_queues = new_pbio_data_queue(1234, 10);

	svc_accept_queues = new_queue(10);

	init_svc_accept();
	init_pbio_link_accept();

	if (pthread_create(&svc_accept_tid, NULL, svc_accept_thread, NULL) < 0) {
		goto done;
	}
	if (pthread_create(&pbio_accept_tid, NULL, pbio_link_accept_thread, NULL) < 0) {
		goto done;
	}
	for (i=0; i<num_workers; i++) {
		if (pthread_create(&svc_worker_tid[i], NULL, svc_worker_thread, NULL) < 0) {
			goto done;
		}
	}

	pthread_join(svc_accept_tid, NULL);
	pthread_join(pbio_accept_tid, NULL);
	for (i=0; i<num_workers; i++) {
		pthread_join(svc_worker_tid[i], NULL);
	}

done:
	free_queue(svc_accept_queues);
	destroy_pbio_data(&pbio_data_queues);

	return 0;
}
