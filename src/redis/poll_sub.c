#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis.h>
#include <async.h>
#include <arpa/inet.h> 
#include <sys/poll.h> 
#include <signal.h>

int running = 1;

void sigfunc(int sig)
{
	running = 0;
}

int main(int argv, char** args)
{
	int num = 10;
	redisReply *reply;
	signal(SIGPIPE, SIG_IGN);

	signal(SIGINT, sigfunc);

	redisContext* c = redisConnect("127.0.0.1", 6379);
	if (c->err) {
		return 1;
	}

	reply = redisCommand(c, "subscribe Q1 Q2 Q3"); 
	if (!reply) {
		return -1;
	}

    c->flags &= ~REDIS_BLOCK;

	struct pollfd pfp[1];
	int status;
	int n = 0;


	while (running) {
		pfp[0].fd = c->fd;
		pfp[0].events = POLLIN;

//		printf("poll: %d, %d\n", n++, sdslen(c->obuf));
		poll(pfp, 1, 1);

		if (pfp[0].revents & POLLIN) {
			redisReply *r;

			if (redisBufferRead(c) == REDIS_ERR) {
				//__redisAsyncDisconnect(ac);
			}
			while((status = redisGetReply(c,(void**)&r)) == REDIS_OK) {
				if (!r) break;
				if (r->type == REDIS_REPLY_ARRAY && r->elements == 3) {
					char *data = r->element[2]->str;
					if (data) {
						printf("%s\n", data);
					}
					freeReplyObject(r);
				}
			}
		}
	}
    freeReplyObject(reply);
    redisFree(c);

	return 0;
}
