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
static struct io_data queue[QUEUE_MAX_SIZE];
int front = 1, rear = -1;

void init_queue(void)
{
	front = rear = 0;
}

void clear_queue(void)
{
	front = rear;
}

int put_queue(struct io_data *data, struct io_index *idx)
{
	if ((rear+1) % QUEUE_MAX_SIZE == front) {    // 큐가 꽉 찼는지 확인
		printf ("Queue Overflow.");
		return -1;
	}
	
	idx->index = rear;

	queue[rear] = data;                  // rear가 큐의 끝 다음의 빈공간이므로 바로 저장
	rear = ++rear % MAXSIZE;             // rear를 다음 빈공간으로 변경
	return 0;
}

struct io_data * get_queue_index(uint16_t index)
{
	if (index >= QUEUE_MAX_SIZE) {
		return NULL;
	}
	return queue[index];
}

struct io_data * get_queue()
{
	struct io_data *data;

	if (front == rear) {                  // 큐가 비어 있는지 확인
		printf ("Queue Underflow.");
		return NULL;
	}

	data = queue[front];    // front의 값을 가져옴
	front = ++front % QUEUE_MAX_SIZE;   // front를 다음 데이터 요소로
	return data;
}


int cio_init(io_link *ctx)
{
	int c_sock;
	c_sock = uds_connect_sock(IO_SO_FILE);
	ctx->sock = c_sock;
}

int cio_write(io_link *ctx)
{
}

int cio_read(io_link *ctx)
{
}

void cio_destory(io_link *ctx)
{
}



int sio_init(io_link *ctx)
{
	int l_sock;
	l_sock = uds_listen_sock(IO_SO_FILE);	
	ctx->sock = l_sock;
}

int sio_write(io_link *ctx)
{
	puts_data();
}

int sio_read(io_link *ctx)
{
}

void sio_destory(io_link *ctx)
{
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
