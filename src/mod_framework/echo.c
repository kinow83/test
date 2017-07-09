/*
 * echo.c
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

int init_echo(void *ctx) {
	printf("call init_echo\n");
	return 0;
}

void deinit_echo(void *ctx) {
	printf("call deinit_echo\n");
}

int run_echo(void *ctx, void *user) {
	printf("call run_echo\n");
	return 0;
}

struct mod mod_echo = {
		.name = "echo",
		.init = init_echo,
		.deinit = deinit_echo,
		.run = run_echo,
};
