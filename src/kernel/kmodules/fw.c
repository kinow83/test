


static struct nf_hook_ops fw_ops[] __read_most = {
	{
		.hook = fw4_filter,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_PRE_ROUTING,,
		.priority = NF_IP_PRI_FILTER,
	},
	{
		.hook = fw_lo_filter,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER,
	},
	{
		.hook = fw_br_filter,
		.pf = NFPROTO_BRIDGE,
		.hooknum = NF_BR_PRE_ROUTING,
		.priority = NF_BR_PRI_BRNF,
	},
};

int init_hook()
{
	int ret;

	ret = nf_register_net_hooks();
}
