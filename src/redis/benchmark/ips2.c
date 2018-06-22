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
static const char **ips_queue_name;
static size_t *ips_queue_name_len;
static char (*proxy_queue_name)[32];
static int thread_num;

static int core_num()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
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
	char peer_qname[64];
	bmctx_t *ctx;
	size_t ctxsize = sizeof(bmctx_t);

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
		reply1 = redisCommandArgv(c, thread_num+2, ips_queue_name, ips_queue_name_len);
		if (reply1) {
			tot_rx++;
			if (reply1->type == REDIS_REPLY_ARRAY && reply1->elements == 2) {
				tot_data_len += reply1->element[1]->len;
				ctx = (bmctx_t*)reply1->element[1]->str;

				// ips0
				int idx = atoi(reply1->element[0]->str+3);
				//printf("ips: %s -> proxy: %s\n", reply1->element[0]->str, proxy_queue_name[idx]);

				reply2 = redisCommand(c, "LPUSH %s %b", proxy_queue_name[idx], ctx, ctxsize);
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
	thread_num = core_num();
	int i;
	pthread_t *tid = malloc(thread_num);

	signal(SIGPIPE, SIG_IGN);
	printf("core = %d\n", thread_num);

	ips_queue_name = (const char **)malloc((thread_num+2) * sizeof(const char**));
	ips_queue_name_len = malloc((thread_num+2) * sizeof(size_t));
	proxy_queue_name = (char (*)[32])calloc(1, thread_num * 32);

	ips_queue_name[0] = strdup("BRPOP");
	ips_queue_name_len[0] = 5;

	for (i=0; i<thread_num; i++) {
		ips_queue_name[i+1] = malloc(32);
		snprintf((char *)ips_queue_name[i+1], 32, "ips%d", i);
		ips_queue_name_len[i+1] = strlen(ips_queue_name[i+1]);

		snprintf(proxy_queue_name[i], 32, "proxy%d", i);
	}

	ips_queue_name[i+1] = strdup("0");
	ips_queue_name_len[i+1] = 1;

	for (i=0; i<thread_num+2; i++) {
		printf("%s %ld\n", ips_queue_name[i], ips_queue_name_len[i]);
	}

	for (i=0; i<thread_num; i++) {
		int *id = malloc(sizeof(int));
		*id = i;
		pthread_create(&tid[i], NULL, worker, id);
	}

	for (i=0; i<thread_num; i++) {
		pthread_join(tid[i], NULL);
	}
}

