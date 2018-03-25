#include <stdio.h>


typedef struct ipsubnet {
	uint32_t ip;
	uint8_t cidr;
} ipsubnet;

#define MAX_SUBNET	0xFFFF
static ipsubnet subnets[MAX_SUBNET];
static int subnet_len = 0;

void add_ipsubnet(ipsubnet *s)
{
	subnet_len = (subnet_len + 1) % MAX_SUBNET;
	subnets[subnet_len] = *s;
}

int match_ipsubnet(uint32_t ip)
{
	int i;
	ipsubnet *s;
	for (i=0; i<subnet_len; i++) {
		s = &subnets[i];
		if (s->cidr == 32) {
			if (ip == s->ip) {
				return 1;
			}
		} else {

		}
	}
}

int main()
{


}
