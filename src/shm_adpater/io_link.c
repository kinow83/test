#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "queue.h"
#include "sock.h"

#define IO_SO_FILE "/tmp/uds.sock"


struct io_data {
	uint8_t unused;
	uint8_t data[1500];
	size_t datalen;
};

struct io_hello {
	uint16_t index;
	struct sockaddr_in from;
} __attribute__((packed));

static io_index {
	uint16_t index;
};

// http://hellmath.tistory.com/8
const int QUEUE_MAX_SIZE = 0xFFFF;
struct io_data *queue;
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

int put_queue(struct io_data *iodata, struct io_index *idx)
{
	if ((rear+1) % QUEUE_MAX_SIZE == front) {    // 큐가 꽉 찼는지 확인
		printf ("Queue Overflow.");
		return -1;
	}
	
	idx->index = rear;

	memcpy(queue[rear].data, iodata->data, iodata->datalen);
	queue[rear].unused = 0;
	queue[rear].datalen = iodata->datalen; // rear가 큐의 끝 다음의 빈공간이므로 바로 저장
	rear = ++rear % MAXSIZE;             // rear를 다음 빈공간으로 변경
	return 0;
}

struct io_data * get_queue_index(uint16_t index)
{
	if (index >= QUEUE_MAX_SIZE) {
		return NULL;
	}
	return &queue[index];
}

struct io_data * get_queue()
{
	struct io_data *iodata;

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

int cio_init(io_link *ctx)
{
	int c_sock;
	c_sock = uds_connect_sock(IO_SO_FILE);
	ctx->sock = c_sock;

	shmid = shmget((key_t)SKEY, sizeof(struct io_data)*QUEUE_MAX_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		shmid = shmget((key_t)SKEY, sizeof(struct io_data)*QUEUE_MAX_SIZE, 0666);
		if (shmid == -1) goto error;
	}

	queue = (struct io_data *)shmat(shmid, (void *)0, 0);

	return 0;

error:
	if (c_sock != -1) close(c_sock);
	return -1;
}

int cio_write(io_link *ctx, void *data)
{
}

int cio_read(io_link *ctx, void *data)
{
}

void cio_destory(io_link *ctx)
{
	if (shmid != -1) shmdt(queue);
}


int sio_init(io_link *ctx)
{
	int l_sock = -1;
	l_sock = uds_listen_sock(IO_SO_FILE);	
	if (l_sock == -1) goto error;
	ctx->sock = l_sock;

	shmid = shmget((key_t)SKEY, sizeof(struct io_data)*QUEUE_MAX_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		shmid = shmget((key_t)SKEY, sizeof(struct io_data)*QUEUE_MAX_SIZE, 0666);
		if (shmid == -1) goto error;
	}

	queue = (struct io_data *)shmat(shmid, (void *)0, 0);

	return 0;

error:
	if (l_sock != -1) close(l_sock);
	return -1;
}

int sio_write(io_link *ctx, void *data)
{
	struct io_index idx;
	struct io_data iodata;
	iodata.data = data;
	iodata.datalen = datalen;

	if (put_queue(iodata, &idx) < 0) {
		return -1;
	}

	write(ctx->sock, &idx,
}

int sio_read(io_link *ctx, void *data)
{
}

void sio_destory(io_link *ctx)
{
	if (shmid != -1) shmdt(queue);
}


extern io_link cio = {
	.init    = cio_init,
	.write   = cio_write,
	.read    = cio_read,
	.destroy = cio_destroy,
};

extern io_link sio = {
	.init    = sio_init,
	.write   = sio_write,
	.read    = sio_read,
	.destroy = sio_destroy,
};
