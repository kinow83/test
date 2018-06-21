#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "benchmark.h"

__thread uint64_t tot_data_len = 0;
__thread uint64_t tot_rx = 0;
__thread uint64_t tot_tx = 0;


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

void *push_worker(void *arg)
{
    redisContext *c;
    redisReply *reply;
	char uds[64];
	char my_qname[64];
	char peer_qname[64];
	bmctx_t ctx;
	bmconfig_t *config = (bmconfig_t*)arg;
	size_t ctxsize = sizeof(bmctx_t);
	char tname[64];

	snprintf(tname, 64, "proxy_thread %d/%u", config->id, config->count);

	get_proxy_queue_name(my_qname, 64, config->id);
	get_ips_queue_name(peer_qname, 64, config->id);
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

	printf("push %s\n", peer_qname);

	init_bm_timediff(&config->td);
	while (1) {
		ctx.data[0] = (int)tot_tx;
		reply = redisCommand(c, "LPUSH %s %b", peer_qname, &ctx, ctxsize);
		if (reply) {
			freeReplyObject(reply);
			tot_tx++;
		}
		if (tot_tx == config->count) {
			break;
		}
	}
	printf("%s ->tx: %lu\n", tname, tot_tx);
    redisFree(c);
    return NULL;
}

void *pop_worker(void *arg)
{
    redisContext *c;
    redisReply *reply;
	char uds[64];
	char my_qname[64];
	char peer_qname[64];
	bmctx_t *ctx;
	bmconfig_t *config = (bmconfig_t*)arg;
	size_t ctxsize = sizeof(bmctx_t);
	char tname[64];

	snprintf(tname, 64, "proxy_thread %d/%u", config->id, config->count);
	get_proxy_queue_name(my_qname, 64, config->id);
	get_ips_queue_name(peer_qname, 64, config->id);
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

	printf("pop %s\n", my_qname);

	while (1) {
		reply = redisCommand(c, "BRPOP %s 0", my_qname);
		if (reply) {
			tot_rx++;
			if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
				tot_data_len += reply->element[1]->len;
			}
			ctx = (bmctx_t *)reply->element[1]->str;
			//printf("%d\n", ctx->data[0]);
			if (tot_rx == config->count) {
				break;
			}
			freeReplyObject(reply);
		}
	}
	check_bm_timediff(tname, &config->td, 1);
    redisFree(c);
    return NULL;
}


int main(int argc, char** argv)
{
	int count = atoi(argv[1]);
	int thread_num = core_num();
	int i;
	pthread_t *push_tid = malloc(thread_num);
	pthread_t *pop_tid = malloc(thread_num);
	bmconfig_t **config = malloc(sizeof(bmconfig_t) * thread_num);

	signal(SIGPIPE, SIG_IGN);

	for (i=0; i<thread_num; i++) {
		config[i] = malloc(sizeof(bmconfig_t));
		config[i]->id = i;
		config[i]->count = count;
	}

	for (i=0; i<thread_num; i++) {
		pthread_create(&pop_tid[i], NULL, pop_worker, config[i]);
	}
	for (i=0; i<thread_num; i++) {
		pthread_create(&push_tid[i], NULL, push_worker, config[i]);
	}

	for (i=0; i<thread_num; i++) {
		pthread_join(push_tid[i], NULL);
		pthread_join(pop_tid[i], NULL);
	}
}

