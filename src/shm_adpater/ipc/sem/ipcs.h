#ifndef _IPCS_
#define _IPCS_

#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


typedef struct shm_data {
	unsigned int type;
	unsigned int value;
} shm_data_t;

struct ipcs_ops {
	void (*create)(void);
	void (*open)(void);
	void (*post)(void);
	void (*wait)(void);
	int (*wait_timeout)(uint32_t msec);
	void (*destroy)(void);
};

#define TEST_LOOP 100000

#endif
