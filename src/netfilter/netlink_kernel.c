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

#define NF_MULTI

#define NETLINK_USER 31

struct sock *nl_sk = NULL;

static void hello_nl_recv_msg(struct sk_buff* skb) {
	char *msg = "Hello from kernel";
	int msg_size = strlen(msg);
	struct nlmsghdr *nlh;
	__u32 pid;
	int res;
	struct sk_buff *skb_out;

	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

	nlh = (struct nlmsghdr*)skb->data;
	printk(KERN_INFO "Netlink recv message palyload: %s\n", (char*)nlmsg_data(nlh));
	pid = nlh->nlmsg_pid; // pid of sending process

	skb_out = nlmsg_new(msg_size, 0);
	if (!skb_out) {
		printk(KERN_ERR "Failed to allocate new skb\n");
		return;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
#ifdef NF_MULTI
	NETLINK_CB(skb_out).dst_group = 1;
#else
	NETLINK_CB(skb_out).dst_group = 0; // not in multicast group
#endif
	strncpy(nlmsg_data(nlh), msg, msg_size);

#ifdef NF_MULTI
	res = nlmsg_multicast(nl_sk, skb_out, 0, 1, GFP_ATOMIC);
#else
	res = nlmsg_multic (nl_sk, skb_out, pid);
#endif
	if (res < 0) {
		printk(KERN_ERR "Error while sending message to user\n");
		return;
	}
	return;
}

static int hello_init(void) {
	printk("Entering: %s\n", __FUNCTION__);

	struct netlink_kernel_cfg cfg = {
			.input = hello_nl_recv_msg,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
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
