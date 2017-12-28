#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "epoller.h"
#include "sock.h"

int read_handler(Epoller *e, void *data)
{
	char buf[1500];
	int r = read(e->sock, buf, sizeof(buf));

	printf("%s\n", buf);
	return r;
}

int timeout_handler(Epoller *e, void *data)
{
	printf("timeout\n");
	return -1;
}

void accept_handler(int sock)
{
	int res;
	Epoller e;
	struct sockaddr_in caddr;
	socklen_t caddrlen = sizeof(caddr);
	int csock = accept(sock, (struct sockaddr*)&caddr, &caddrlen);

	e.sock = csock;
	e.timeout = 5000;
	e.timeout_handler = timeout_handler;
	e.read_handler = read_handler;
	
	res = epoller_wait(&e, NULL);
}

int main()
{
	int sock = tcp_listen_sock(8888);
	accept_handler(sock);
	return 0;
}
