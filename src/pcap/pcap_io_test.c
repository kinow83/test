#include <sys/time.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <pcap/pcap.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TIMEDIFF(d, t1, t2) \
do { \
    (d)->tv_sec  = t2.tv_sec  - t1.tv_sec; \
	(d)->tv_usec = t2.tv_usec - t1.tv_usec; \
	if ((d)->tv_usec < 0) { \
		(d)->tv_sec--; \
		(d)->tv_usec += 1000000; \
    } \
} while(0)

struct pcap_io_tester {
	void (*init) (void *ctx);
	int  (*read) (void *ctx, const uint8_t* buf, size_t buflen);
	int  (*write)(void *ctx, const uint8_t* buf, size_t buflen);
	void (*close)(void *ctx);
};

struct pcap_io_ctx {
	uint16_t port;
	const pcap_t *pcap;
	const struct pcap_io_tester *tester;
};


void print_hex(const uint8_t *buf, size_t buflen)
{
	int i;
	for (i=0; i<buflen; i++) {
		if (i%8 == 0) {
			printf(" ");
			if (i%16 == 0) {
				printf("\n");
			}
		}
		printf("%02x ", buf[i]);
	}
	printf("(%ld)\n", buflen);
}


/*
 * typedef void (*pcap_handler)(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes);
 */

static void pcap_cb(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
	struct ether_header *ep;
	struct ip *iph;
	struct tcphdr *tcph;
	uint16_t ether_type;
	uint16_t sport, dport;
	struct pcap_io_ctx *ctx;
	const struct pcap_io_tester *tester;
	size_t len;
	const uint8_t *p = packet;


	ctx = (struct pcap_io_ctx *)user;
	tester = ctx->tester;

	ep = (struct ether_header *)p;

	p += sizeof(struct ether_header);

	ether_type = ntohs(ep->ether_type);
	if (ether_type != ETHERTYPE_IP) {
		return;
	}
	iph = (struct ip*)p;
	if (iph->ip_p != IPPROTO_TCP) {
		return;
	}
	tcph = (struct tcphdr*)(p + iph->ip_hl*4);
	if (!tcph->psh) return;
//	if (tcph->syn) return;
//	if (tcph->fin) return;
	sport = ntohs(tcph->source);
	dport = ntohs(tcph->dest);

	p = (uint8_t*)tcph + sizeof(struct tcphdr);
//	printf("%s\n", p);

//	printf("%ld: %ld.%ld\n", time(NULL), pkthdr->ts.tv_sec, pkthdr->ts.tv_usec);
	len = pkthdr->len - (p - packet);
	if (dport == ctx->port) {
#if 0
		print_hex(packet, pkthdr->len);
		tester->read(ctx, (const uint8_t*)p, len);
#endif
	} else {
		print_hex(packet, pkthdr->len);
		tester->write(ctx, (const uint8_t*)p, len);
	}
}

static void pcap_timeloop(pcap_t *pcap, pcap_handler callback, u_char *user)
{
	int res;
	const u_char *packet;
	struct pcap_pkthdr *header;
	struct timeval t0, t1, t2, td;
	int devnull = open("/dev/null", O_RDONLY);

first:
	res = pcap_next_ex(pcap, &header, &packet);
	if (res <= 0) {
		goto first;
	} else {
		t1.tv_sec  = header->ts.tv_sec;
		t1.tv_usec = header->ts.tv_usec;
		t0 = t1;
		callback(user, header, packet);
	}

	while ((res = pcap_next_ex(pcap, &header, &packet)) >= 0) {
		TIMEDIFF(&t2, t1, header->ts);
		TIMEDIFF(&td, t0, header->ts);

//		printf("\t sleep: %ld.%ld\n", t2.tv_sec, t2.tv_usec);
//		printf("\t sleep: %ld.%ld\n", td.tv_sec, td.tv_usec);

		while (select(devnull, NULL, NULL, NULL, &t2) != 0);
		callback(user, header, packet);

		t1 = header->ts;
	}

	close(devnull);
}

static pcap_t *new_pcap(const char *filename, const char *filter)
{
	pcap_t *pcap = NULL;
	struct bpf_program  fcode;
	char errbuf[PCAP_ERRBUF_SIZE];

	pcap = pcap_open_offline(filename, errbuf);
	if (!pcap) {
		fprintf(stderr, "pcap_open_offline - %s\n", errbuf);
		return NULL;
	}

	if (filter && filter[0]) {
		if (pcap_compile(pcap, &fcode, filter, 1, PCAP_NETMASK_UNKNOWN) < 0) {
			fprintf(stderr, "pcap_compile - %s\n", filter);
			pcap_close(pcap);
			return NULL;
		}

		if (pcap_setfilter(pcap, &fcode) < 0) {
			fprintf(stderr, "pcap_setfilter - %s\n", filter);
			pcap_close(pcap);
			return NULL;
		}
	}

	return	pcap;
}


static void pcap_io_test_init(void *ctx)
{
	return;
}

static int pcap_io_test_read(void *ctx, const uint8_t* buf, size_t buflen)
{
	char buffer[1500];
	snprintf(buffer, sizeof(buffer) < buflen+1 ? sizeof(buffer) : buflen+1, "%s", buf);
	printf("[%ld] C: %s\n", buflen, (char*)buffer);
	return buflen;
}

static int pcap_io_test_write(void *ctx, const uint8_t* buf, size_t buflen)
{
	char buffer[1500];
	snprintf(buffer, sizeof(buffer) < buflen+1 ? sizeof(buffer) : buflen+1, "%s", buf);
	printf("[%ld] S: %s\n", buflen, (char*)buffer);
	return buflen;
}

static void pcap_io_test_close(void *ctx)
{
	return;
}

static struct pcap_io_tester pcap_tester = {
	.init  = pcap_io_test_init,
	.read  = pcap_io_test_read,
	.write = pcap_io_test_write,
	.close = pcap_io_test_close,
};



int main(int argc, char **argv)
{
	pcap_t *pcap;
	char *pcapfile;
	char *filter;
	char *port;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_io_ctx ctx;

	pcapfile = argv[1];
	filter   = argv[2];
	port     = argv[3];

	pcap = new_pcap(pcapfile, filter);
	if (pcap == NULL) {
		exit(1);
	}

	ctx.port   = atoi(port);
	ctx.pcap   = pcap;
	ctx.tester = &pcap_tester;

//	pcap_loop(pcap, -1, pcap_cb, NULL);
	pcap_timeloop(pcap, pcap_cb, (void *)&ctx);

	pcap_close(pcap);
}
