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
#include <signal.h>
#include <assert.h>
#include <sys/prctl.h>

#include "dprintf.h"
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

static svc_portmap_t g_svc_configs[3] = {
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
typedef struct g_pbio_listen_t {
	char     svc_name[64]; // service name
	int      svc_lsock;    // service listen sock
	uint16_t svc_lport;	   // service listen port

	int      proxy_lsock; // proxy listen sock
	int      proxy_csock; // proxy client sock
	char     proxy_uds_sofile[128]; // proxy uds sock file
} g_pbio_listen_t;

#define NUM_PBIO_LISTEN 1500
static g_pbio_listen_t g_pbio_listen[NUM_PBIO_LISTEN];
static pthread_mutex_t g_pbio_listen_lock = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t g_worker_wakeup_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_worker_wakeup_cond = PTHREAD_COND_INITIALIZER;

/* [accept sock queue] ************************/
static pthread_mutex_t g_svc_accept_queue_lock = PTHREAD_MUTEX_INITIALIZER;
static queue_t*        g_svc_accept_queue;

/* [number of listen sock] ************************/
static int g_num_svc_listen;
static int g_num_proxy_listen;

/* [] ************************/
typedef struct pbio_trx_t {
	xid_t xid;
	uint16_t index;
} __attribute__((packed)) pbio_trx_t;

/* [] ************************/
static pbio_data_queue_t *g_pbio_data_queue;
static pthread_mutex_t      g_pbio_data_queue_lock = PTHREAD_MUTEX_INITIALIZER;

static int doing = 1;
static void signal_handler(int signo)
{
	doing = 0;
}

/* [service worker thread] ************************/
static void *svc_worker_thread(void *arg)
{
	int proxy_csock, rc;
	size_t nbytes;
	svc_sock_t *svc_sock = NULL;
	pbio_trx_t pbio_trx;
	pbio_data_t *pbio_data;
	struct timespec timeout;

	prctl(PR_SET_NAME, "pb_svc_worker", 0, 0, 0);

retry:
	/* accquire accept client sock */
	while (doing) {
		svc_sock = NULL;
		timeout.tv_sec = time(NULL);
		timeout.tv_nsec = 1000*1000;
		// wakeup worker thread
		if (pthread_cond_timedwait(&g_worker_wakeup_cond, &g_worker_wakeup_lock, &timeout) < 0) {
			if (errno == ETIMEDOUT) {
				printf(".");
				continue;
			}
			continue;
		}
		svc_sock = dequeue(g_svc_accept_queue);
		pthread_mutex_unlock(&g_svc_accept_queue_lock);
		break;
	}
	if (!svc_sock) {
		goto retry;
	}

	if (gen_xid(&pbio_trx.xid, &svc_sock->caddr)) {
		goto retry;
	}

	/* check proxy client connection */
	pthread_mutex_lock(&g_pbio_listen_lock);
	{
		proxy_csock = g_pbio_listen[svc_sock->lsock].proxy_csock;
	}
	pthread_mutex_unlock(&g_pbio_listen_lock);
	if (proxy_csock <= 0) {
		DPRINTF("'%s' proxy client doesn't connected. can't service. closed\n", 
				g_pbio_listen[svc_sock->lsock].svc_name);
		close(svc_sock->csock); // close client
		free(svc_sock);
		goto retry;
	}

	// service client
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
	while (doing) {
		res = epoll_wait(efd, &ev, 1, 100);
		if (res < 0) {
			if (errno == EINTR) continue;
			break;
		} else if (res != 1) {
			continue;
		}

		pthread_mutex_lock(&g_pbio_data_queue_lock);
		{
			pbio_data = alloc_pbio_data_queue(g_pbio_data_queue, &pbio_trx.index);
		}
		pthread_mutex_unlock(&g_pbio_data_queue_lock);
		if (!pbio_data) {
			DPRINTF("proxy client too busy....\n");
			break;
		}
		DPRINTF("alloc: %p, index:%d\n", pbio_data, pbio_trx.index);

		// write client's data to pbio data.
		nbytes = read(svc_sock->csock, pbio_data->data, PBIO_DATA_SIZE);
		if (nbytes < 0) {
			if (errno == EINTR) continue;
		}
		if (nbytes <= 0) {
			break;
		}
		pbio_data->datalen = nbytes;

		DPRINTF("%s (%ld)\n", pbio_data->data, pbio_data->datalen);

		// notify pbio data (index) to proxy client
		nbytes = write(proxy_csock, &pbio_trx, sizeof(pbio_trx));
		if (nbytes < 0) {
			if (errno == EINTR) continue;
		}
		if (nbytes <= 0) {
			close(proxy_csock);
			pthread_mutex_lock(&g_pbio_listen_lock);
			{
				g_pbio_listen[svc_sock->lsock].proxy_csock = -1;
			}
			pthread_mutex_unlock(&g_pbio_listen_lock);
			break;
		}
	}
	close(svc_sock->csock); // close client
	free(svc_sock);
	goto retry;

	return NULL;
}

static void finish_svc_listen()
{
	int i, svc_lsock;
	for (i=0; i<NUM_PBIO_LISTEN; i++) {
		svc_lsock = g_pbio_listen[i].svc_lsock;
		if (svc_lsock == i && svc_lsock > 0) {
			close(svc_lsock);
		}
		g_pbio_listen[i].svc_lsock = -1;
	}
}

static void finish_proxy_listen()
{
	int i, svc_lsock;
	int proxy_csock, proxy_lsock;
	for (i=0; i<NUM_PBIO_LISTEN; i++) {
		svc_lsock   = g_pbio_listen[i].svc_lsock;
		proxy_lsock = g_pbio_listen[i].proxy_lsock;
		proxy_csock = g_pbio_listen[i].proxy_csock;
		if (svc_lsock == i) {
			if (proxy_lsock > 0) {
				close(proxy_lsock);
			}
			if (proxy_csock > 0) {
				close(proxy_csock);
			}
		}
		g_pbio_listen[i].proxy_lsock = -1;
		g_pbio_listen[i].proxy_csock = -1;
	}
}

static void init_svc_listen()
{
	int i;
	int svc_lsock, svc_lport;
	int num_svc_portmap = sizeof(g_svc_configs) / sizeof(svc_portmap_t);

	for (i=0; i<num_svc_portmap; i++) {
		g_pbio_listen[i].svc_lsock = -1;
	}

	g_num_svc_listen = 0;
	for (i=0; i<num_svc_portmap; i++) {
		svc_lport = g_svc_configs[i].svc_port;
		svc_lsock = tcp_listen_sock(svc_lport);
		pthread_mutex_lock(&g_pbio_listen_lock);
		{
			g_pbio_listen[svc_lsock].svc_lsock = svc_lsock;
			g_pbio_listen[svc_lsock].svc_lport = svc_lport;
			snprintf(g_pbio_listen[svc_lsock].svc_name,          64, "%s", g_svc_configs[i].svc_name);
			snprintf(g_pbio_listen[svc_lsock].proxy_uds_sofile, 128, "%s", g_svc_configs[i].proxy_uds_sofile);
			DPRINTF("%s: listen svc: sock: %d, port: %d\n", g_svc_configs[i].svc_name, svc_lsock, svc_lport);
		}
		pthread_mutex_unlock(&g_pbio_listen_lock);
		g_num_svc_listen++;
	}
}

static void init_proxy_listen()
{
	int i;
	int svc_lsock, proxy_lsock;
	const char *proxy_uds_sofile;

	for (i=0; i<NUM_PBIO_LISTEN; i++) {
		g_pbio_listen[i].proxy_lsock = -1;
		g_pbio_listen[i].proxy_csock = -1;
	}

	g_num_proxy_listen = 0;
	for (i=0; i<NUM_PBIO_LISTEN; i++) {
		pthread_mutex_lock(&g_pbio_listen_lock);
		{
			svc_lsock = g_pbio_listen[i].svc_lsock;
			if (svc_lsock == i) {
				proxy_lsock = uds_listen_sock(g_pbio_listen[svc_lsock].proxy_uds_sofile);
				if (proxy_lsock < 0) {
					perror("uds_listen_sock()");
					exit(1);
				}
				g_pbio_listen[i].proxy_lsock = proxy_lsock;
				DPRINTF("%s(%d): listen pbio: listen:%d, uds:%s\n", 
					g_pbio_listen[i].svc_name, i, proxy_lsock, g_pbio_listen[i].proxy_uds_sofile);
			}
		}
		pthread_mutex_unlock(&g_pbio_listen_lock);
		g_num_proxy_listen++;
	}
}

static int set_pbio_listen_proxy_csock(int _proxy_lsock, int proxy_csock)
{
	int i;
	int svc_lsock, proxy_lsock;
	for (i=0; i<NUM_PBIO_LISTEN; i++) {
		svc_lsock   = g_pbio_listen[i].svc_lsock;
		proxy_lsock = g_pbio_listen[i].proxy_lsock;
		if ((svc_lsock == i) && (proxy_lsock == _proxy_lsock)) {
			g_pbio_listen[i].proxy_csock = proxy_csock;
			return 0;
		}
	}
	return -1;
}

/* [accept for proxy client] ************************/
static void *proxy_accept_thread(void *arg)
{
	int i, k, res, ret;
	int efd = -1;
	int proxy_lsock = -1, proxy_csock = -1;
	int svc_lsock = -1;
	struct epoll_event ev, *evs = NULL;
	struct sockaddr_un caddr;
	socklen_t caddrlen = sizeof(caddr);
	int nbytes;
	pbio_trx_t pbio_trx;
	int num_epoll = g_num_proxy_listen * 2;
	int max_proxy_lsock = -1;
	uint32_t idle = 0;

	prctl(PR_SET_NAME, "pb_proxy_accept", 0, 0, 0);

	efd = epoll_create(num_epoll);
	if (efd < 0) {
		goto done;
	}
	evs = (struct epoll_event*)malloc(sizeof(struct epoll_event) * num_epoll);
	for (i=0; i<NUM_PBIO_LISTEN; i++) {
		svc_lsock = g_pbio_listen[i].svc_lsock;
		if (svc_lsock == i) {
			proxy_lsock = g_pbio_listen[i].proxy_lsock;
			ev.events   = EPOLLIN;
			ev.data.fd  = proxy_lsock;
			if (max_proxy_lsock < proxy_lsock) {
				max_proxy_lsock = proxy_lsock;
			}
			if (epoll_ctl(efd, EPOLL_CTL_ADD, proxy_lsock, &ev) < 0) {
				perror("epoll_ctl:ADD");
				goto done;
			}
			DPRINTF("%s added pbio list accept epoll, sock:%d\n", g_pbio_listen[i].svc_name, proxy_lsock);
		}
	}

retry:
	while (doing) {
		res = epoll_wait(efd, evs, num_epoll, 100);
		if (res < 0) {
			if (errno == EINTR) continue;
			break;
		} else if (res == 0) {
			idle++;
			if (idle > 10) {
				pthread_mutex_lock(&g_svc_accept_queue_lock);
				{
					gc_unused_pbio_data_queue(g_pbio_data_queue, 3);
				}
				pthread_mutex_unlock(&g_svc_accept_queue_lock);
			}
			continue;
		} else {
			idle = 0;
		}
		for (i=0; i<res; i++) {
			// accept proxy client
			if (evs[i].data.fd <= max_proxy_lsock) {
				proxy_lsock = evs[i].data.fd;
				proxy_csock = accept(proxy_lsock, (struct sockaddr*)&caddr, &caddrlen);

				// epoll add proxy client
				ev.events  = EPOLLIN;
				ev.data.fd = proxy_csock;
				if (epoll_ctl(efd, EPOLL_CTL_ADD, proxy_csock, &ev) < 0) {
					perror("epoll_ctl:ADD");
					break;
				}

				pthread_mutex_lock(&g_svc_accept_queue_lock);
				{
					assert(set_pbio_listen_proxy_csock(proxy_lsock, proxy_csock) == 0);
					DPRINTF("%s pbio link connected... (proxy_csock:%d)\n", 
						g_pbio_listen[i].svc_name, proxy_csock);
				}
				pthread_mutex_unlock(&g_svc_accept_queue_lock);
			}
			// read proxy client
			else {
				proxy_csock = evs[i].data.fd;
				nbytes = read(proxy_csock, &pbio_trx, sizeof(pbio_trx));
				if (nbytes < 0) {
					if (errno == EINTR) continue;
					close(proxy_csock);
					for (k=0; k<NUM_PBIO_LISTEN; k++) {
						if (g_pbio_listen[k].proxy_csock == proxy_csock) {
							DPRINTF("%s:%d proxy connect error.\n", 
								g_pbio_listen[k].svc_name, g_pbio_listen[k].proxy_csock);
							g_pbio_listen[k].proxy_csock = -1;
							break;
						}
					}
					goto retry;
				}
				else if (sizeof(pbio_trx) != nbytes) {
					close(proxy_csock);
					for (k=0; k<NUM_PBIO_LISTEN; k++) {
						if (g_pbio_listen[k].proxy_csock == proxy_csock) {
							DPRINTF("%s:%d proxy connect closed.\n", 
								g_pbio_listen[k].svc_name, g_pbio_listen[k].proxy_csock);
							g_pbio_listen[k].proxy_csock = -1;
							break;
						}
					}
					goto retry;
				}
				dealloc_pbio_data_queue(g_pbio_data_queue, pbio_trx.index);
			}
		}
	}
done:
	if (evs > 0) free(evs);
	if (efd > 0) close(efd);
	return NULL;
}

/* [service accept thread] ************************/
static void *svc_accept_thread(void *arg)
{
	int i, res;
	int svc_lsock, svc_csock;
	int efd = -1;
	struct epoll_event ev, *evs = NULL;
	svc_sock_t *svc_sock;
	struct sockaddr_in caddr;
	socklen_t caddrlen = sizeof(caddr);

	prctl(PR_SET_NAME, "pb_svc_accept", 0, 0, 0);

	efd = epoll_create(g_num_svc_listen);
	if (efd < 0) {
		goto done;
	}
	evs = (struct epoll_event*)malloc(sizeof(struct epoll_event) * g_num_svc_listen);
	for (i=0; i<NUM_PBIO_LISTEN; i++) {
		svc_lsock = g_pbio_listen[i].svc_lsock;
		if (svc_lsock == i) {
			ev.events = EPOLLIN;
			ev.data.fd = svc_lsock;
			if (epoll_ctl(efd, EPOLL_CTL_ADD, svc_lsock, &ev) < 0) {
				perror("epoll_ctl:ADD");
				goto done;
			}
			DPRINTF("%s added svc accept epoll, sock:%d\n", g_pbio_listen[i].svc_name, svc_lsock);
		}
	}

	while (doing) {
		res = epoll_wait(efd, evs, g_num_svc_listen, 100);
		if (res < 0) {
			if (errno == EINTR) continue;
			break;
		} else if (res == 0) {
			// timeout
			continue;
		}
		for (i=0; i<res; i++) {
			svc_lsock = evs[i].data.fd;
			svc_csock = accept(svc_lsock, (struct sockaddr*)&caddr, &caddrlen);
			DPRINTF("%s connected...\n", g_pbio_listen[svc_lsock].svc_name);

			svc_sock = malloc(sizeof(*svc_sock));
			svc_sock->lsock = svc_lsock;
			svc_sock->csock = svc_csock;
			memcpy(&svc_sock->caddr, &caddr, sizeof(caddr));

			pthread_mutex_lock(&g_svc_accept_queue_lock);
			{
				if (enqueue(g_svc_accept_queue, svc_sock) < 0) {
					DPRINTF("svc accept queue is FULL!!!\n");
				}
			}
			pthread_mutex_unlock(&g_svc_accept_queue_lock);
			pthread_cond_broadcast(&g_worker_wakeup_cond);
		}
	}
done:
	if (evs > 0) free(evs);
	if (efd > 0) close(efd);
	return NULL;
}

static void init_signal()
{
	signal(SIGINT, signal_handler);
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

	g_pbio_data_queue = new_pbio_data_queue(1234, 10, 1);

	g_svc_accept_queue = new_queue(10);

	init_svc_listen();
	init_proxy_listen();

	if (pthread_create(&svc_accept_tid, NULL, svc_accept_thread, NULL) < 0) {
		goto done;
	}
	if (pthread_create(&pbio_accept_tid, NULL, proxy_accept_thread, NULL) < 0) {
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
	finish_svc_listen();
	finish_proxy_listen();
	free_queue(g_svc_accept_queue);
	destroy_pbio_data(&g_pbio_data_queue);

	return 0;
}
