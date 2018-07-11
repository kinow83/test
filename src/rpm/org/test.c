#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eap.h"

int main()
{
	libeap_t *eap = new_eap();
	printf("%p\n", eap->ctx);

	free(eap);
}
