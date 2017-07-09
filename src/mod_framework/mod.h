/*
 * mod.h
 *
 *  Created on: 2017. 7. 9.
 *      Author: root
 */

#ifndef SRC_MOD_FRAMEWORK_MOD_H_
#define SRC_MOD_FRAMEWORK_MOD_H_

struct mod {
	void *ctx;
	char *name;
	int (*init)(void *ctx);
	void (*deinit)(void *ctx);
	int (*run)(void *ctx, void *user);
};

#endif /* SRC_MOD_FRAMEWORK_MOD_H_ */
