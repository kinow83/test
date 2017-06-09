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

#define MY_GROUP 0x33

#define MY_PROTO NETLINK_USERSOCK

#define MAX_PAYLOAD 1024

#ifdef MY_GROUP
	const char *head = "multicast";
#else
	const char *head = "unicast";
#endif


static int open_netlink_socket(void) {
	int sock;
	struct sockaddr_nl saddr;

	sock = socket(PF_NETLINK, SOCK_RAW, MY_PROTO);
	if (sock < 0) {
		perror("socket()");
		return -1;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.nl_family = AF_NETLINK;
	saddr.nl_pid = getpid(); /* self pid */
#ifdef MY_GROUP
	saddr.nl_groups = MY_GROUP;
#else
	saddr.nl_groups = 0; /* not in mcast groups */
#endif

	if (bind(sock, (struct sockaddr*)&saddr, sizeof(saddr))) {
		perror("bind()");
		close(sock);
		return -1;
	}
	return sock;
}

static int send_netlink_msg(int sock, const char *message) {
	struct iovec iov;
	struct msghdr msg;
	struct nlmsghdr *nlh;
	struct sockaddr_nl daddr;
	int bytes;

	nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	 /* Fill the netlink message header */
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;
	 /* Fill in the netlink message payload */
	strcpy(NLMSG_DATA(nlh), message);

	memset(&iov, 0, sizeof(iov));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&daddr, 0, sizeof(daddr));
	daddr.nl_family = AF_NETLINK;
	daddr.nl_pid = 0; // for linux kernel
#ifdef MY_GROUP
	daddr.nl_groups = MY_GROUP;
#else
	daddr.nl_groups = 0; /* not in mcast groups */
#endif

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&daddr;
	msg.msg_namelen = sizeof(daddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	printf("[%s] Sending message to kernel => %s\n", head, message);
	bytes = sendmsg(sock, &msg, 0);
	if (bytes <= 0) {
		perror("sendmsg()");
		free(nlh);
		return -1;
	}
	free(nlh);
	return bytes;
}

static int recv_netlink_msg(int sock) {
	struct iovec iov;
	struct msghdr msg;
	int bytes;
	struct sockaddr_nl daddr;
	struct nlmsghdr *nlh;

	nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

	memset(&iov, 0, sizeof(iov));
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&daddr;
	msg.msg_namelen = sizeof(daddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	printf("[%s] Waiting for message from kernel\n", head);

	bytes = recvmsg(sock, &msg, 0);
	if (bytes <= 0) {
		perror("recvmsg()");
		free(nlh);
		return -1;
	}
	printf("[%s] recv message addr\n", head);
	printf("\tpid   = %u\n", daddr.nl_pid);
	printf("\tgroup = %u\n", daddr.nl_groups);
	printf("\trecv message payload(%d): %s\n", bytes, (char *)NLMSG_DATA(nlh));
	free(nlh);
	return bytes;
}



int main() {
	int sock;
	char buf[MAX_PAYLOAD];

	sock = open_netlink_socket();
	if (sock < 0) {
		exit(1);
	}

	snprintf(buf, sizeof(buf), "%u say hello", getpid());
	if (send_netlink_msg(sock, buf) < 0) {
		close(sock);
		exit(1);
	}

	while (recv_netlink_msg(sock) > 0) {
	}

	close(sock);
	return 0;
}

