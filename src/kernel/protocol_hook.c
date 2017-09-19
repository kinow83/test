/*
 * protocol_hook.c
 *
 *  Created on: 2016. 10. 13.
 *      Author: root
 */




#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/netdevice.h>
#include <net/protocol.h>


static int my_rcv(struct sk_buff *skb)
{
	struct net *net = dev_net(skb->dev);

	pr_info("owner:%d, ", net->user_ns->owner.val);
	pr_info("group:%d, ", net->user_ns->group.val);
	pr_info("[Kernel] Packet received with length: %u\n", skb->len);
	return skb->len;
}

static struct net_protocol my_protocol = {
		.early_demux = NULL,
		.err_handler = NULL,
		.handler = my_rcv,
		.no_policy = 1,
		.netns_ok = 1
};

int init_module(void)
{
	if (inet_add_protocol( &my_protocol, IPPROTO_TCP ) < 0) {
		pr_crit("[Kernel] failure add my protocol hook\n");
		return -1;
	} else {
		pr_info("[Kernel] success add my protocol hook\n");
	}
	return 0;
}

void cleanup_module( void )
{
	if (inet_del_protocol( &my_protocol, IPPROTO_TCP ) < 0) {
		pr_crit("[Kernel] failure del my protocol hook\n");
	} else {
		pr_info("[Kernel] success del my protocol hook\n");
	}
	return;
}


MODULE_LICENSE( "GPL" );


