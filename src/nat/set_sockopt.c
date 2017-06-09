/*
 * set_sockopt.c
 *
 *  Created on: 2017. 6. 9.
 *      Author: root
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <error.h>
#include <stdio.h>
#include "nat_sockopt.h"

int main(int argc, char **argv) {
	struct nat_so_opt_t opt;
	int sz = sizeof(opt);
	int sock;


	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket()");
		exit(1);
	}

	opt.id = 100;
	snprintf(opt.name, sizeof(opt.name), "kaka1000");
	if (setsockopt(sock, IPPROTO_IP, NAT_SO_GET_NAME, &opt, (socklen_t)sz) < 0) {
		perror("getsockopt()");
		close(sock);
		exit(1);
	}
	close(sock);
	return 0;
}
