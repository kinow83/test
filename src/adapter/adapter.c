#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include "sock.h"

#define MAX_EVENTS 1024

static int stop = 0;
const char *terminate = "close";
size_t      terminatelen = 5;

static void die(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static int next_sock(int *list, size_t size, int start)
{
	int s = start;
	int sock = 0;
	while (1) {
		sock = list[start++ % size];
		if (sock) {
			break;
		}
	}
	printf("\tselect %d=>%d, %d\n", s, sock, size);
	return sock;
}

void show_sock(const char *msg, int *list, size_t size)
{
	int i;
	for (i=0; i<size; i++) {
		if (list[i])
			printf("%s: [%d]%d\n", msg, i, list[i]);
	}
}

int add_echo_sock(int *list, size_t size, int sock)
{
	int i;
	for (i=0; i<size; i++) {
		if (list[i] == 0) {
			list[i] = sock;
			return i+1;
		}
	}
	return 0;
}

void del_echo_sock(int *list, size_t size, int sock)
{
	int i;
	for (i=0; i<size; i++) {
		if (list[i] == sock) {
			break;
		}
	}
	for (i++; i<size && list[i] !=0; i++) {
		list[i-1] = list[i];
		list[i] = 0;
	}
}

#define SOCK_SIZE 1024
void* worker_thread(void *arg)
{
	int id = (int)arg;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
    char buffer[2048];
	int timeout = -1;
	int echo_listener;
	int client_listener;
	int echo_count = 0;
	int echo_conlist[SOCK_SIZE] = {0, };
	int client_conlist[SOCK_SIZE] = {0, };
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof client_addr;
	int i, nfd, client;
	int to, n;
	int epfd;
	char echo_sock_name[64];
	char client_sock_name[64];

	stop = 0;


    if ((epfd = epoll_create(MAX_EVENTS)) < 0) {
        die("epoll_create");
    }

	sprintf(echo_sock_name, "/tmp/echo_%d.sock", id);
	sprintf(client_sock_name, "/tmp/client_%d.sock", id);

	unlink(echo_sock_name);
	unlink(client_sock_name);

	printf("%s\n", echo_sock_name);
	printf("%s\n", client_sock_name);
    echo_listener   = uds_listen_sock(echo_sock_name);
    client_listener = uds_listen_sock(client_sock_name);

    memset(&ev, 0, sizeof ev);
    ev.events = EPOLLIN;
    ev.data.fd = echo_listener;
    epoll_ctl(epfd, EPOLL_CTL_ADD, echo_listener, &ev);
    ev.data.fd = client_listener;
    epoll_ctl(epfd, EPOLL_CTL_ADD, client_listener, &ev);

    while (stop == 0) {
		nfd = epoll_wait(epfd, events, MAX_EVENTS, timeout);
		for (i=0; i<nfd; i++) {
			if (events[i].data.fd == echo_listener) {
				client = accept(echo_listener, (struct sockaddr *) &client_addr, &client_addr_len);
				if (client < 0) {
					perror("accept");
					continue;
				}

				memset(&ev, 0, sizeof ev);
				ev.events = EPOLLIN; // | EPOLLERR;
				ev.data.fd = client;
				epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);

				echo_count = add_echo_sock(echo_conlist, SOCK_SIZE, client);
#ifdef DEBUG
				printf("accept echo = %d/%d\n", client, echo_count);
				show_sock("accept echo", echo_conlist, SOCK_SIZE);
#endif
			}
			else if (events[i].data.fd == client_listener) {
				client = accept(client_listener, (struct sockaddr *) &client_addr, &client_addr_len);
				if (client < 0) {
					perror("accept");
					continue;
				}

				memset(&ev, 0, sizeof ev);
				ev.events = EPOLLIN; // | EPOLLERR;
				ev.data.fd = client;
				epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);

				client_conlist[client] = client;
#ifdef DEBUG
				printf("accept client = %d\n", client);
				show_sock("accept client", client_conlist, SOCK_SIZE);
#endif
			}
			else {
				client = events[i].data.fd;
				if (client_conlist[client] == client) {
					n = read(client, buffer, sizeof(buffer));
					if (n <= 0) {
						perror("error, client read");
						epoll_ctl(epfd, EPOLL_CTL_DEL, client, NULL);
						close(client);
						client_conlist[client] = 0;
					} else {
						to = echo_conlist[client % echo_count];
						n = write(to, &client, sizeof(client));
						if (n <= 0) {
							perror("error, echo write");
							epoll_ctl(epfd, EPOLL_CTL_DEL, to, NULL);
							close(to);
							del_echo_sock(echo_conlist, SOCK_SIZE, to);
						} else {
#ifdef DEBUG
							printf("client:%d -> echo:%d\n", client, to);
#endif
						}
					}
				}
				else { //if (echo_conlist[client] == client) {
					n = read(client, &to, sizeof(to));
					if (n <= 0) {
						perror("error, client read");
						epoll_ctl(epfd, EPOLL_CTL_DEL, client, NULL);
						close(client);
						del_echo_sock(echo_conlist, SOCK_SIZE, client);
					} else {
						n = write(to, buffer, sizeof(buffer));
						if (n <= 0) {
							perror("error, echo write");
							epoll_ctl(epfd, EPOLL_CTL_DEL, to, NULL);
							close(to);
							client_conlist[to] = 0;
						} else {
#ifdef DEBUG
							printf("echo:%d -> client:%d\n", client, to);
#endif
						}
					}
				}

			}
		}
	}
	close(echo_listener);
	close(client_listener);
	close(epfd);
    return NULL;
}

int main(int argc, char **argv)
{
	int thread_num = atoi(argv[1]);
	pthread_t *tid = malloc(sizeof(pthread_t) * thread_num);
	int i;

	for (i=0; i<thread_num; i++) {
		pthread_create(&tid[i], NULL, worker_thread, (void *)i);
	}
	for (i=0; i<thread_num; i++) {
		pthread_join(tid[i], NULL);
	}
    return 0;
}
