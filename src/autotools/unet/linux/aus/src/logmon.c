#include <stdio.h>
#include <stdlib.h>
#include "eap.h"

int main(int argc, char **argv) {

	printf("this is logmon\n");

	libeap_t *eap = new_eap();


	free(eap);

}
