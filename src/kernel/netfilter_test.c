/*
 * netfilter_test.c
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


 /* IP Hooks */
 /* After promisc drops, checksum checks. */
 #define NF_IP_PRE_ROUTING   0
 /* If the packet is destined for this box. */
 #define NF_IP_LOCAL_IN      1
 /* If the packet is destined for another interface. */
 #define NF_IP_FORWARD       2
 /* Packets coming from a local process. */
 #define NF_IP_LOCAL_OUT     3
 /* Packets about to hit the wire. */
 #define NF_IP_POST_ROUTING  4
 #define NF_IP_NUMHOOKS      5

unsigned int hook_sample(void *priv,
	       struct sk_buff *skb,
	       const struct nf_hook_state *state)
{
	struct iphdr *iph;
	struct tcphdr *tcph;


	if (skb) {
		iph = ip_hdr(skb);

		if (iph && (iph->protocol == IPPROTO_TCP)) {


			tcph = (struct tcphdr *)((__u32*)iph + iph->ihl);

//			printk("recv tcp %d -> %d\n", ntohs(tcph->source), ntohs(tcph->dest));

			if ((ntohs(tcph->source) == 80) || (ntohs(tcph->source) == 443)) {
				printk("ip: %d.%d.%d.%d\n",
						((__u8*)&iph->saddr)[0],
						((__u8*)&iph->saddr)[1],
						((__u8*)&iph->saddr)[2],
						((__u8*)&iph->saddr)[3]);
				return NF_DROP;
			}
		}
	}

	return NF_ACCEPT;
}


static struct nf_hook_ops nfilter = {
		.hook     = hook_sample,
		.pf       = PF_INET,
		.hooknum  = NF_IP_LOCAL_IN,
		.priority = NF_IP_PRI_FIRST
};

int nfilter_init(void)
{
	nf_register_hook( &nfilter );
	printk("[kernel] NFilter Init.\n");
	return 0;
}

void nfilter_exit(void)
{
	nf_unregister_hook( &nfilter );
	printk("[kernel] NFilter Exit.\n");
}

MODULE_LICENSE( "GPL" );
module_init(nfilter_init);
module_exit(nfilter_exit);



