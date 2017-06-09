#include <net/ethernet.h>


/* 10Mb/s ethernet header */    
struct ether_header{
  struct ether_addr dst;    /* destination eth addr */
  struct ether_addr src;    /* source ether addr    */
  uint16_t ether_type;      /* packet type ID field */
} __attribute__ ((__packed__));

// ethernet.h
struct ether_addr
{
  u_int8_t ether_addr_octet[ETH_ALEN];
} __attribute__ ((__packed__));

struct ether_addr multicast_ether_addr(const struct in6_addr *addr)
{
	struct ether_addr e;
	int i;

	// IPv6 Multicast ethernet address: 3333 xxxx xxxx
	e.ether_addr_octet[0] = 0x33;
	e.ether_addr_octet[1] = 0x33;

	for (i=2; i<6; i++) {
		e.ether_addr_octet[i] = addr->s6_addr[i+10];
	}
	return e;
}


bool multicast_scan(const char *ptarget)
{
	uint8_t buffer[65556];
	struct ether_header *ether;
	uint8_t *ipv6buffer;
	struct ip6_hdr *ipv6;
	struct in6_addr target;

	ether = (struct ether_header *)buffer;
	ipv6buffer = buffer + sizeof(struct ether_header);
	ipv6 = (struct ip6_hdr *)v6buffer;


	if (inet_pton(AF_INET6, ptarget, target) <= 0) {
		return false;
	}

	
}


int main(int argc, char **argv)
{


}
