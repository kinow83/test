/*
 * netlink_user.c
 *
 *  Created on: 2017. 5. 29.
 *      Author: root
 */


#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#define NF_MULTI

#define NETLINK_USER 31

#define MAX_PAYLOAD 1024

int main() {

	int sock_fd;
	struct nlmsghdr *nlh;
	struct sockaddr_nl src_addr, dst_addr;
	struct iovec iov;
	struct msghdr msg;

	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if (sock_fd < 0) {
		perror("socket()");
		return -1;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();

	if (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr))) {
		perror("bind()");
		close(sock_fd);
		return -1;
	}

	memset(&dst_addr, 0, sizeof(dst_addr));
	dst_addr.nl_family = AF_NETLINK;
	dst_addr.nl_pid = 0; // for linux kernel
#ifdef NF_MULTI
	dst_addr.nl_groups = 1; // group 1 multicast
#else
	dst_addr.nl_groups = 0; // unicast
#endif

	nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	strcpy(NLMSG_DATA(nlh), "Hello! I'm userspace");

	memset(&iov, 0, sizeof(iov));
	memset(&msg, 0, sizeof(msg));

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dst_addr;
	msg.msg_namelen = sizeof(dst_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	printf("Sending message to kernel\n");
	if (sendmsg(sock_fd, &msg, 0) <= 0) {
		perror("sendmsg()");
		close(sock_fd);
		return -1;
	}
	printf("Waiting for message from kernel\n");

	while (1) {
		recvmsg(sock_fd, &msg, 0);
		printf("recv message payload: %s\n", (char *)NLMSG_DATA(nlh));
	}
	close(sock_fd);
	return 0;
}

