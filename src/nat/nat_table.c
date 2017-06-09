/*
 * nat_table.c
 *
 *  Created on: 2017. 6. 9.
 *      Author: root
 */

#include <net/sock.h>
#include <linux/netfilter.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <net/compat.h>
#include <net/sock.h>
#include <linux/netdevice.h>
#include "nat_sockopt.h"

static struct nat_so_opt_t gopt;

static int do_nat_set_ctl(struct sock *sk, int cmd, void __user* user, unsigned int len) {
	int ret;
	struct nat_so_opt_t opt;

	if ( !ns_capable(sock_net(sk)->user_ns, CAP_NET_ADMIN) )
		return -EPERM;

	printk("%s cmd = %d\n", __FUNCTION__, cmd);

	if (len != sizeof(opt)) {
		return -EINVAL;
	}

	if (copy_from_user(&opt, user, sizeof(opt)) != 0) {
		return -EFAULT;
	}

	switch (cmd) {
	case NAT_SO_SET_TEST:
		// noop
		break;
	case NAT_SO_SET_NAME:
		gopt.id = opt.id;
		snprintf(gopt.name, sizeof(gopt.name), "%s", opt.name);
		ret = 0;

		printk("NAT_SO_SET_NAME> id=%d, name=%s\n", gopt.id, gopt.name);
		break;
	default:
		ret = -EINVAL;
	}

    return ret;
}

static int do_ef_get_ctl(struct sock *sk, int cmd, void __user *user, int *len) {
	int ret;
	struct nat_so_opt_t opt;

	if ( !ns_capable(sock_net(sk)->user_ns, CAP_NET_ADMIN) )
		return -EPERM;

	printk("%s cmd = %d\n", __FUNCTION__, cmd);

	if (*len != sizeof(opt)) {
		return -EINVAL;
	}

	switch (cmd) {
	case NAT_SO_GET_TEST:
		// noop
		break;
	case NAT_SO_GET_NAME:
		if (copy_to_user(user, &gopt, *len) != 0) {
			return -EFAULT;
		}
		ret = 0;
		break;
	default:
		ret = -EINVAL;
	}

    return ret;
}

static struct nf_sockopt_ops nat_sockopts = {
		.list = {0, 0},
		.pf = PF_INET,
		.owner = THIS_MODULE,
		.set_optmin = NAT_BASE_CTL,
		.set_optmax = NAT_SO_SET_MAX,
		.set = do_nat_set_ctl,
		.get_optmin = NAT_BASE_CTL,
		.get_optmax = NAT_SO_GET_MAX,
		.get = do_ef_get_ctl,

};

static int __init nat_init(void) {
	int ret;
	printk("nat table init\n");


	ret = nf_register_sockopt(&nat_sockopts);
	if (ret < 0) {
		printk("Unable to register sockopts.\n");
		return ret;
	}

	return 0;
}

static void __exit nat_exit(void) {
	printk("nat table exit\n");
	nf_unregister_sockopt(&nat_sockopts);
}

module_init(nat_init);
module_exit(nat_exit);

