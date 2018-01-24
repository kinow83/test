#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tls.h"


libtls_t *new_tls() {
	printf("call new_tls\n");
	return malloc(sizeof(libtls_t));
}
