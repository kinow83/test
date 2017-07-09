/*
 * mod.c
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


extern struct mod mod_hello;
extern struct mod mod_echo;


static struct mod *mods[] = {
		&mod_hello, &mod_echo,
};

static void print_mods() {
	struct mod **m;
	int i;

	for (i=1, m = mods;
			*m != NULL;
			m++) {
		printf("%s\n", m[0]->name);
	}
}

static int run_mod(const char *name, void *ctx, void *user) {
	struct mod **m;
	for (m = mods;
			*m != NULL;
			m++) {
		if (strcmp(name, m[0]->name) == 0) {
			if ((m[0]->init(ctx)) < 0) {
				return -1;
			}
			if ((m[0]->run(ctx, user)) < 0) {
				m[0]->deinit(ctx);
				return -1;
			}
			m[0]->deinit(ctx);
		}
	}
	return -1;
}

int main(int argc, char *argv[]) {
	const char *name;

	if (argc < 2) {
		exit(1);
	}

	name = argv[1];

	return run_mod(name, NULL, NULL);
}
