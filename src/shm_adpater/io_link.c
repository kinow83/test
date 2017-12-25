#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/shm.h> 
#include <errno.h> 

#include "queue.h"
#include "sock.h"
#include "io_link.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#define IO_SO_FILE "/tmp/uds.sock"

#define IO_DATA_SIZE 1500

typedef struct iodata_t {
	uint8_t unused;
	uint8_t data[IO_DATA_SIZE];
	size_t datalen;
} iodata_t;


/* [ queue ] ***********************************************/
// http://hellmath.tistory.com/8
const int QUEUE_MAX_SIZE = 0xFFFF;
static iodata_t *ioqueue;
static int front = -1;
static int rear  = -1;

static void init_queue(void)
{
	front = rear = 0;
}

static void clear_queue(void)
{
	front = rear;
}

static int put_queue(const void *data, size_t datalen, ioindex_t *ioindex)
{
	iodata_t *iodata;

	if ((rear+1) % QUEUE_MAX_SIZE == front) {    // 큐가 꽉 찼는지 확인
		printf ("Queue Overflow.");
		return -1;
	}
	
	ioindex->index = rear;

	iodata = &ioqueue[rear];

	memcpy(iodata->data, data, datalen);
	iodata->unused = 1;
	iodata->datalen = datalen;

	// rear를 다음 빈공간으로 변경
	rear = ++rear % QUEUE_MAX_SIZE;
	return 0;
}

iodata_t * get_queue(uint16_t index)
{
	if (index >= QUEUE_MAX_SIZE) {
		return NULL;
	}
//	ioqueue[index].unused = 0;
	return &ioqueue[index];
}


#if 0
iodata_t * pop_queue()
{
	iodata_t *iodata;

	if (front == rear) {                  // 큐가 비어 있는지 확인
		printf ("Queue Underflow.");
		return NULL;
	}

	iodata = &ioqueue[front];    // front의 값을 가져옴
	front = ++front % QUEUE_MAX_SIZE;   // front를 다음 데이터 요소로
	return iodata;
}
#endif
/* end [ queue ] ***********************************************/


static int shmid = -1;
#define SKEY 1234


/* [ disposer ] ***********************************************/
static int io_disposer_init(void *ctx);
static int io_disposer_pop (void *ctx, uint8_t *data, size_t *datalen, ioindex_t *ioindex);
static int io_disposer_mark(void *ctx, ioindex_t *ioindex);
static int io_disposer_send(void *ctx, ioindex_t *ioindex);
static int io_disposer_recv(void *ctx, ioindex_t *ioindex);
static void io_disposer_destroy(void *ctx);

static int io_disposer_init(void *_ctx)
{
	io_disposer_ctx *ctx = (io_disposer_ctx *)_ctx;
	int sock;
	sock = uds_connect_sock(IO_SO_FILE);
	if (sock < 0) {
		return -1;
	}
	ctx->connect_sock = sock;

	shmid = shmget((key_t)SKEY, sizeof(iodata_t)*QUEUE_MAX_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		shmid = shmget((key_t)SKEY, sizeof(iodata_t)*QUEUE_MAX_SIZE, 0666);
		if (shmid == -1) {
			goto error;
		}
	}

	ioqueue = (iodata_t *)shmat(shmid, (void *)0, 0);

	return 0;

error:
	if (sock < 0) {
		close(sock);
	}
	io_disposer_destroy(ctx);
	return -1;
}

static int io_disposer_pop (void *_ctx, uint8_t *data, size_t *datalen, ioindex_t *ioindex)
{
	iodata_t * iodata = get_queue(ioindex->index);
	if (iodata->unused == 1) {
		return -1;
	}
	data = iodata->data;
	*datalen = iodata->datalen;
	return 0;
}

static int io_disposer_mark(void *_ctx, ioindex_t *ioindex)
{
	iodata_t * iodata = get_queue(ioindex->index);
	if (iodata->unused == 0) {
		return -1;
	}
	iodata->unused = 0;
	return 0;
}

static int io_disposer_send(void *_ctx, ioindex_t *ioindex)
{
	io_disposer_ctx *ctx = (io_disposer_ctx *)_ctx;
	int ret;
	while (1) {
		ret = write(ctx->connect_sock, ioindex, sizeof(*ioindex));
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			}
			perror("disposer_send:write");
			return -1;
		}
		else if (ret == 0) {
			return -1;
		}
		else if (ret != sizeof(ioindex)) {
			return -1;
		}
		else {
			break;
		}
	}
	return 0;
}

static int io_disposer_recv(void *_ctx, ioindex_t *ioindex)
{
	io_disposer_ctx *ctx = (io_disposer_ctx *)_ctx;
	int ret;
	while (1) {
		ret = read(ctx->connect_sock, ioindex, sizeof(*ioindex));
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			}
			perror("disposer_recv:read");
			return -1;
		}
		else if (ret == 0) {
			return -1;
		}
		else if (ret != sizeof(ioindex)) {
			return -1;
		}
		else {
			break;
		}
	}
	return 0;
}

static void io_disposer_destroy(void *_ctx)
{
	io_disposer_ctx *ctx = (io_disposer_ctx *)_ctx;
	if (shmid != -1) {
		shmdt(ioqueue);
	}
	if (ctx->connect_sock > 0) {
		close(ctx->connect_sock);
	}
}
/* end [ disposer ] ***********************************************/


/* [ offerer ] ***********************************************/
static int io_offerer_init(void *ctx);
static int io_offerer_put (void *ctx, uint8_t *data, size_t datalen, ioindex_t *ioindex);
static int io_offerer_send(void *ctx, ioindex_t *ioindex);
static int io_offerer_recv(void *ctx, ioindex_t *ioindex);
static void io_offerer_destroy(void *ctx);

static int io_offerer_init(void *_ctx)
{
	io_offerer_ctx *ctx = (io_offerer_ctx *)_ctx;
	int sock;
	sock = uds_listen_sock(IO_SO_FILE);
	if (sock < 0) {
		return -1;
	}
	ctx->listen_sock = sock;

	shmid = shmget((key_t)SKEY, sizeof(iodata_t)*QUEUE_MAX_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		shmid = shmget((key_t)SKEY, sizeof(iodata_t)*QUEUE_MAX_SIZE, 0666);
		if (shmid == -1) {
			goto error;
		}
	}

	ioqueue = (iodata_t *)shmat(shmid, (void *)0, 0);

	return 0;

error:
	if (sock < 0) {
		close(sock);
	}
	io_offerer_destroy(ctx);
	return -1;
}

static int io_offerer_put(void *_ctx, uint8_t *data, size_t datalen, ioindex_t *ioindex)
{
	if (put_queue(data, datalen, ioindex) < 0) {
		return -1;
	}
	return 0;
}

static int io_offerer_send(void *_ctx, ioindex_t *ioindex)
{
	io_offerer_ctx *ctx = (io_offerer_ctx *)_ctx;
	int ret;
	while (1) {
		ret = write(ctx->accept_sock, ioindex, sizeof(*ioindex));
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			}
			perror("offerer_send:write");
			return -1;
		}
		else if (ret == 0) {
			return -1;
		}
		else if (ret != sizeof(ioindex)) {
			return -1;
		}
		else {
			break;
		}
	}
	return 0;
}

static int io_offerer_recv(void *_ctx, ioindex_t *ioindex)
{
	io_offerer_ctx *ctx = (io_offerer_ctx *)_ctx;
	int ret;
	while (1) {
		ret = read(ctx->accept_sock, ioindex, sizeof(*ioindex));
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			}
			perror("offerer_recv:read");
			return -1;
		}
		else if (ret == 0) {
			return -1;
		}
		else if (ret != sizeof(ioindex)) {
			return -1;
		}
		else {
			break;
		}
	}
	return 0;
}

static void io_offerer_destroy(void *_ctx)
{
	io_offerer_ctx *ctx = (io_offerer_ctx *)_ctx;
	if (shmid != -1) {
		shmdt(ioqueue);
	}
	if (ctx->accept_sock > 0) {
		close(ctx->accept_sock);
	}
	if (ctx->listen_sock > 0) {
		close(ctx->listen_sock);
	}
}

iolinker io_disposer = {
	.init    = io_disposer_init,
	.put     = NULL,
	.pop     = io_disposer_pop,
	.mark    = io_disposer_mark,
	.send    = io_disposer_send,
	.recv    = io_disposer_recv,
	.destroy = io_disposer_destroy,
};

iolinker io_offerer = {
	.init    = io_offerer_init,
	.put     = io_offerer_put,
	.pop     = NULL,
	.mark    = NULL,
	.send    = io_offerer_send,
	.recv    = io_offerer_recv,
	.destroy = io_offerer_destroy,
};
