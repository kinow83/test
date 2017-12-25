#include <stdio.h>
#include <string.h>
#include <stdint.h>

struct foo {
	int a;
	union {
		int b;
		char c[128];
	};
};

typedef struct svc_portmap_t {
	const char *name;
	const uint16_t svc_port;
	const uint16_t proxy_port;
} svc_portmap_t;


typedef struct f1_t {
	char a;
	int b;
} __attribute__((packed)) f1_t;

typedef struct f2_t {
	f1_t a;
	char b;
} __attribute__((packed)) f2_t;

static svc_portmap_t svc_portmaps[2] = {
	{.name="aaa", .svc_port=1, .proxy_port=2},
};


int main()
{
	struct foo f;

	f.a = 10;
	f.b = 20;
	strcpy(f.c, "kaka");

	f1_t f1;
	f2_t f2;
	printf("f1_t = %ld\n", sizeof(f1));
	printf("f2_t = %ld\n", sizeof(f2));
}
