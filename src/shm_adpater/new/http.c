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

#define HTTP_UDS_SOFILE "/tmp/http.sock"

/* [] ************************/
typedef struct pbio_link_data_t {
	xid_t xid;
	uint16_t index;
} __attribute__((packed)) pbio_link_data_t;

/* [] ************************/
static pbio_data_queue_t *pbio_data_queues;

int main(int argc, char **argv)
{
	int ssock;
	int i, nbytes;
	pbio_link_data_t pbio_link_data;

	pbio_data_queues = new_pbio_data_queue(1234, 10);

retry:
	while (1) {
		ssock = uds_connect_sock(HTTP_UDS_SOFILE);
		if (ssock <= 0) {
			usleep(100);
			continue;
		}
		break;
	}

	while (1) {
		nbytes = read(ssock, &pbio_link_data, sizeof(pbio_link_data));
		if (nbytes <= 0) {
			if (errno == EINTR) continue;
			close(ssock);
			goto retry;
		}
		printf("wakeup: xid:%u, index:%d\n", pbio_link_data.xid.hash, pbio_link_data.index);
	}

done:

	return 0;
}
