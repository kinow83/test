/*
 * netlink_kernel.c
 *
 *  Created on: 2017. 5. 29.
 *      Author: root
 */

#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

/**
 * for
 * https://www.systutorials.com/docs/linux/man/7-netlink/
 *
 */

#define MY_GROUP 0x33

#define MY_PROTO NETLINK_USERSOCK

#ifdef MY_GROUP
	const char *head = "multicast";
#else
	const char *head = "unicast";
#endif

struct sock *nl_sk = NULL;

static void nl_recv_and_send(struct sk_buff* skb) {
	const char *message = "I'm Kernel";
	size_t message_len = strlen(message);
	struct nlmsghdr *nlh;
	__u32 pid;
	int res;
	struct sk_buff *skb_out;

	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

	nlh = (struct nlmsghdr*)skb->data;
	printk(KERN_INFO "[%s] Netlink recv pid = %u\n", head, nlh->nlmsg_pid);
	printk(KERN_INFO "[%s] Netlink recv len = %u\n", head, nlh->nlmsg_len);
	printk(KERN_INFO "[%s] Netlink recv message palyload: %s\n", head, (char*)nlmsg_data(nlh));

	pid = nlh->nlmsg_pid; // pid of sending process

	skb_out = nlmsg_new(message_len, 0);
	if (!skb_out) {
		printk(KERN_ERR "Failed to allocate new skb\n");
		return;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, message_len, 0);
#ifdef MY_GROUP
	NETLINK_CB(skb_out).dst_group = MY_GROUP;
#else
	NETLINK_CB(skb_out).dst_group = 0; // not in multicast group
#endif
	strncpy(nlmsg_data(nlh), message, message_len);

#ifdef MY_GROUP
	res = nlmsg_multicast(nl_sk, skb_out, 0, 1, GFP_ATOMIC);
	//res = netlink_broadcast(nl_sk, skb_out, 0, 1, GFP_KERNEL);
#else
	res = netlink_unicast(nl_sk, skb_out, pid, 0 /*MSG_DONTWAIT*/);
#endif
	if (res < 0) {
		printk(KERN_ERR "Error while sending message to user\n");
		return;
	}
	printk(KERN_INFO "[%s] Netlink send message = %s\n", head, message);
	return;
}

static int hello_init(void) {
	printk("[%s] Entering: %s\n", head, __FUNCTION__);

	struct netlink_kernel_cfg cfg = {
#ifdef MY_GROUP
			.groups = MY_GROUP,
#endif
			.input = nl_recv_and_send,
	};

	///////////////////////////////////////////////////////////////////
	// netlink_kernel_create
	///////////////////////////////////////////////////////////////////
	nl_sk = netlink_kernel_create(&init_net, MY_PROTO, &cfg);
	if (!nl_sk) {
		printk(KERN_ALERT "Error creating socket\n");
		return -10;
	}
	return 0;
}

static void hello_exit(void) {
	printk(KERN_INFO "exit hello_module\n");
	netlink_kernel_release(nl_sk);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
