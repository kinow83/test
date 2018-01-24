#include <stdio.h>
#include <stdlib.h>
#include <pcap/pcap.h>
#include "tls.h"

int main(int argc, char **argv) {
	pcap_t *pcap;
	printf("this is aus\n");
	libtls_t *tls = new_tls();


	free(tls);
}
