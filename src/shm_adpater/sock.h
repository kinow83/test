#ifndef _MY_SOCK_
#define _MY_SOCK_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int uds_connect_sock(const char *sofile);

int uds_listen_sock(const char *sofile);

int tcp_listen_sock(int port);

#endif
