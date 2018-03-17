#include <stdio.h>


typedef struct ipsubnet {
	uint32_t ip;
	uint8_t cidr;
} ipsubnet;

static ipsubnet subnets[128];
static int subnet_len = 0;

void add_ipsubnet(ipsubnet *s)
{
	subnets[subnet_len++] = *s;
}

int match_ipsubnet(uint32_t ip)
{

}

int main()
{


}
