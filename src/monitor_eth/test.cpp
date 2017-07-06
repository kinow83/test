#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "mon_eth.h"

static void *notify_handler(const notify_node_t *arg)
{
	notify_node_t *notify = (notify_node_t*)arg;
	notify_node_print("call notify handler ==>", notify);
	return NULL;
}

int main(int argc, char**argv)
{
	monitoring("eth0", notify_handler);
}
