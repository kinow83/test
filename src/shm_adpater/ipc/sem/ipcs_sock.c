#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ipcs.h"


static int initilize = 0;

const char *sock_file = "shm_uds.sock";
int sockfd = -1;
int clifd = -1;

static void create_sock(void)
{
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	socklen_t client_len;

	if (initilize == 0) {

		if (access(sock_file, F_OK) == 0) {
			unlink(sock_file);
		}
		if ((sockfd = socket(PF_FILE, SOCK_STREAM, 0)) < 0) {
			perror("sock");
			exit(1);
		}
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sun_family = AF_UNIX;
		strncpy(server_addr.sun_path, sock_file, strlen(sock_file));

		if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
			perror("bind");
			exit(1);
		}

		if (listen(sockfd, 5) < 0) {
			perror("listen");
			exit(1);
		}
		printf("listen...\n");

		clifd = accept(sockfd,  (struct sockaddr *)&client_addr, &client_len);
		printf("accept...\n");

		initilize = 1;
	}
}

static void open_sock(void)
{
	struct sockaddr_un server_addr;

	if ((clifd = socket(PF_FILE, SOCK_STREAM, 0)) < 0) {
		perror("sock");
		exit(1);
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	strncpy(server_addr.sun_path, sock_file, strlen(sock_file));

	if (connect(clifd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
		perror("connect");
		exit(1);
	}
	printf("connect ok\n");

}

static int dummy;

static void post_sock(void)
{
	if (write(clifd, &dummy, sizeof(dummy)) != sizeof(dummy)) {
		perror("write");
		exit(1);
	}
}

static void wait_sock(void)
{
	if (read(clifd, &dummy, sizeof(dummy)) != sizeof(dummy)) {
		perror("read");
		exit(1);
	}
}

static int wait_timeout_sock(uint32_t msec)
{
	return 0;
}

static void destroy_sock()
{
	if (sockfd > 0) {
		close(sockfd);
		sockfd = -1;
	}
	if (clifd > 0) {
		close(clifd);
		clifd = -1;
	}
}

struct ipcs_ops ipc = {
	.create = create_sock,
	.open = open_sock,
	.post = post_sock,
	.wait = wait_sock,
	.wait_timeout = wait_timeout_sock,
	.destroy = destroy_sock,
};

