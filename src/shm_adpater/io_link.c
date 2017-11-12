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

#include "queue.h"
#include "sock.h"
#include "io_link.h"

#define IO_SO_FILE "/tmp/uds.sock"


// http://hellmath.tistory.com/8
const int QUEUE_MAX_SIZE = 0xFFFF;
io_data *queue;
static int front = -1;
static int rear  = -1;

/* queue **********************************************************************************/
void init_queue(void)
{
	front = rear = 0;
}

void clear_queue(void)
{
	front = rear;
}

int put_queue(void *data, size_t datalen, io_index *idx)
{
	io_data *iodata;

	if ((rear+1) % QUEUE_MAX_SIZE == front) {    // 큐가 꽉 찼는지 확인
		printf ("Queue Overflow.");
		return -1;
	}
	
	idx->index = rear;

	iodata = &queue[rear];

	memcpy(iodata->data, data, datalen);
	iodata->unused = 0;
	iodata->datalen = datalen;

	// rear를 다음 빈공간으로 변경
	rear = ++rear % QUEUE_MAX_SIZE;
	return 0;
}

io_data * get_queue_index(uint16_t index)
{
	if (index >= QUEUE_MAX_SIZE) {
		return NULL;
	}
	return &queue[index];
}

io_data * get_queue()
{
	io_data *iodata;

	if (front == rear) {                  // 큐가 비어 있는지 확인
		printf ("Queue Underflow.");
		return NULL;
	}

	iodata = &queue[front];    // front의 값을 가져옴
	front = ++front % QUEUE_MAX_SIZE;   // front를 다음 데이터 요소로
	return iodata;
}

/* end queue **********************************************************************************/

static int shmid = -1;
#define SKEY 1234

int cio_init(io_link *ctx)
{
	int c_sock;
	c_sock = uds_connect_sock(IO_SO_FILE);
	if (c_sock < 0) {
		return -1;
	}
	ctx->sock = c_sock;

	shmid = shmget((key_t)SKEY, sizeof(io_data)*QUEUE_MAX_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		shmid = shmget((key_t)SKEY, sizeof(io_data)*QUEUE_MAX_SIZE, 0666);
		if (shmid == -1) goto error;
	}

	queue = (io_data *)shmat(shmid, (void *)0, 0);

	return 0;

error:
	if (c_sock != -1) close(c_sock);
	return -1;
}

static int cio_write_data(io_link *ctx, uint8_t *data, size_t datalen)
{
	return 0;
}

static int cio_read_data(io_link *ctx, uint8_t *data)
{
	return 0;
}

static int cio_read_index(io_link *ctx, io_index *ioindex)
{
	int ret = read(ctx->sock, ioindex, sizeof(*ioindex));
	if (ret < 0) {
		perror("cio_read_index:read");
		return -1;
	}
	else if (ret != sizeof(ioindex)) {
		return -1;
	}
	return 0;

}

static int cio_write_index(io_link *ctx, io_index *ioindex)
{
	return 0;
}

static void cio_destroy(io_link *ctx)
{
	if (shmid != -1) shmdt(queue);
	if (ctx->sock > 0) close(ctx->sock);
}


static int sio_init(io_link *ctx)
{
	int l_sock = -1;
	l_sock = uds_listen_sock(IO_SO_FILE);
	if (l_sock == -1) goto error;
	ctx->sock = l_sock;

	shmid = shmget((key_t)SKEY, sizeof(io_data)*QUEUE_MAX_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		shmid = shmget((key_t)SKEY, sizeof(io_data)*QUEUE_MAX_SIZE, 0666);
		if (shmid == -1) goto error;
	}

	queue = ( io_data *)shmat(shmid, (void *)0, 0);

	return 0;

error:
	if (l_sock != -1) close(l_sock);
	return -1;
}

static int sio_write_index(io_link *ctx, io_index *ioindex)
{
	int ret = write(ctx->sock, ioindex, sizeof(*ioindex));
	if (ret <= 0) {
		return -1;
	}
	return 0;
}

static int sio_write_data(io_link *ctx, uint8_t *data, size_t datalen)
{
	io_index ioindex;

	if (put_queue(data, datalen, &ioindex) < 0) {
		return -1;
	}
	if (sio_write_index(ctx, &ioindex) < 0) {
		// TODO: rollback queue data
		return -1;
	}

	return 0;
}

static int sio_read_data(io_link *ctx, uint8_t *data)
{
	return 0;
}

static int sio_read_index(io_link *ctx, io_index *ioindex)
{
	return 0;
}

static void sio_destroy(io_link *ctx)
{
	if (shmid != -1) shmdt(queue);
	if (ctx->sock > 0) close(ctx->sock);
}


io_linker cio_linker = {
	.init         = cio_init,
	.write_data   = cio_write_data,
	.read_data    = cio_read_data,
	.write_index  = sio_write_index,
	.read_index   = sio_read_index,
	.destroy      = cio_destroy,
};

io_linker sio_linker = {
	.init         = sio_init,
	.write_data   = sio_write_data,
	.read_data    = sio_read_data,
	.write_index  = sio_write_index,
	.read_index   = sio_read_index,
	.destroy      = sio_destroy,
};
