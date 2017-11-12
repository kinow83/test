#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>

int tcp_listen_sock(int port)
{
	struct sockaddr_in s_addr;
	int s_sock;
	int on = 1;

	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = INADDR_ANY;
	s_addr.sin_port = htons(port);

	s_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (s_sock < 0) {
		perror("tcp_listen_sock:socket()");
		return -1;
	}

	setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if (bind(s_sock, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0) {
		close(s_sock);
		perror("tcp_listen_sock:bind()");
		return -1;
	}
	if (listen(s_sock, 5) < 0) {
		close(s_sock);
		perror("tcp_listen_sock:listen()");
		return -1;
	}
	return s_sock;
}

int uds_listen_sock(const char *sofile)
{
	struct sockaddr_un s_addr;
	int s_sock;
	int on = 1;

	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sun_family = AF_UNIX;
	strcpy(s_addr.sun_path, sofile);

	unlink(sofile);

	s_sock = socket(PF_FILE, SOCK_STREAM, 0);
	if (s_sock < 0) {
		perror("uds_listen_sock:socket()");
		return -1;
	}

	setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if (bind(s_sock, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0) {
		perror("uds_listen_sock:bind()");
		close(s_sock);
		return -1;
	}
	if (listen(s_sock, 5) < 0) {
		perror("uds_listen_sock:listen()");
		close(s_sock);
		return -1;
	}
	return s_sock;
}

int uds_connect_sock(const char *sofile)
{
	struct sockaddr_un s_addr;
	int c_sock;

	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sun_family = AF_UNIX;
	strcpy(s_addr.sun_path, sofile);

	c_sock = socket(PF_FILE, SOCK_STREAM, 0);
	if (c_sock < 0) {
		perror("socket()");
		return -1;
	}
	if (connect(c_sock, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0) {
		perror("listen()");
		close(c_sock);
		return -1;
	}
	return c_sock;
}
