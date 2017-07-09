/*
 * hello.c
 *
 *  Created on: 2017. 7. 9.
 *      Author: root
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mod.h"

int init_hello(void *ctx) {
	printf("call init_hello\n");
	return 0;
}

void deinit_hello(void *ctx) {
	printf("call deinit_hello\n");
}

int run_hello(void *ctx, void *user) {
	printf("call run_hello\n");
	return 0;
}

struct mod mod_hello = {
		.name = "hello",
		.init = init_hello,
		.deinit = deinit_hello,
		.run = run_hello,
};
