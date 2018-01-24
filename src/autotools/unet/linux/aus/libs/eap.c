#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eap.h"


libeap_t *new_eap() {
	printf("call new_eap\n");
	return malloc(sizeof(libeap_t));
}
