#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "benchmark.h"

static uint64_t tot_data_len = 0;
static uint64_t tot_rx = 0;
static uint64_t tot_tx = 0;

static int core_num()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

static void get_ips_queue_name(char *buf, size_t len, int id)
{
	snprintf(buf, len, "ips_%d", id);
}

static void get_proxy_queue_name(char *buf, size_t len, int id)
{
	snprintf(buf, len, "proxy_%d", id);
}

void *worker(void *arg)
{
	int id = *(int *)arg;
    redisContext *c;
    redisReply *reply1;
    redisReply *reply2;
	char uds[64];
	char my_qname[64];
	char peer_qname[64];
	bmctx_t *ctx;
	size_t ctxsize = sizeof(bmctx_t);

	get_ips_queue_name(my_qname, 64, id);
	get_proxy_queue_name(peer_qname, 64, id);
	c = redisConnectUnix("/var/run/redis/redis.sock");
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

	while (1) {
		reply1 = redisCommand(c, "BRPOP %s 0", my_qname);
		if (reply1) {
			tot_rx++;
			if (reply1->type == REDIS_REPLY_ARRAY && reply1->elements == 2) {
				tot_data_len += reply1->element[1]->len;
				ctx = (bmctx_t*)reply1->element[1]->str;

				reply2 = redisCommand(c, "LPUSH %s %b", peer_qname, ctx, ctxsize);
				if (reply2) {
					if (reply2->type == REDIS_REPLY_ERROR) {
						printf("reply2 error - %s\n", reply2->str);
						freeReplyObject(reply2);
						break;
					}
					tot_tx++;
					freeReplyObject(reply2);
				} else {
					printf("reply2 null\n");
				}
			} else if (reply1->type == REDIS_REPLY_ERROR) {
				printf("reply1 error - %s\n", reply1->str);
			} else {
				printf("reply1 error\n");
			}
			freeReplyObject(reply1);
		} else {
			printf("reply1 null\n");
		}
	}
    redisFree(c);
    return NULL;
}

int main(int argc, char** argv)
{
	int thread_num = core_num();
	int i;
	pthread_t *tid = malloc(thread_num);

	signal(SIGPIPE, SIG_IGN);
	printf("core = %d\n", thread_num);

	for (i=0; i<thread_num; i++) {
		int *id = malloc(sizeof(int));
		*id = i;
		pthread_create(&tid[i], NULL, worker, id);
	}

	for (i=0; i<thread_num; i++) {
		pthread_join(tid[i], NULL);
	}
}

