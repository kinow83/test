/*
 * resolv.c
 *
 *  Created on: 2017. 8. 10.
 *      Author: root
 */

/**
 * ref:
 * http://lethean.github.io/2011/04/05/get-domain-mx-host-name/
 */

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>

static char *lookup_mx(const char *name)
{
	unsigned char response[NS_PACKETSZ]; /* big enough, right? */
	ns_msg handle;
	int ns_index;
	int len;

	len = res_search(name, C_IN, T_MX, response, sizeof(response));
	if (len < 0) {
		/* failed to search MX records */
		return strdup(name);
	}
	if (ns_initparse(response, len, &handle) < 0) {
		/* failed to parse MX records for '%s'", name); */
		return strdup(name);
	}
	len = ns_msg_count(handle, ns_s_an);
	if (len <= 0) {
		/* no mx records */
		return strdup(name);
	}
	for (ns_index = 0; ns_index < len; ns_index++) {
		ns_rr rr;
		char dispbuf[4096];

		if (ns_parserr(&handle, ns_s_an, ns_index, &rr)) {
			/* WARN: ns_parserr failed */
			continue;
		}
		ns_sprintrr(&handle, &rr, NULL, NULL, dispbuf, sizeof(dispbuf));
		if (ns_rr_class (rr) == ns_c_in && ns_rr_type (rr) == ns_t_mx) {
			char mxname[MAXDNAME];

			dn_expand(ns_msg_base(handle),
			ns_msg_base (handle) + ns_msg_size(handle),
			ns_rr_rdata(rr) + NS_INT16SZ, mxname, sizeof(mxname));
			printf("MX=%s\n", mxname);
			//return strdup(mxname);
		}
	}
	return strdup(name);
}


int main(int argc, char **argv)
{
	char *target;


	target = lookup_mx(argv[1]);
	printf("%s\n", target);

	free(target);
	return 0;
}
