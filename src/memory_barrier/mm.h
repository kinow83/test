#ifndef __MM_MM__
#define __MM_MM__

struct ctx {
	int a;
	char data[1500];
	int c;
	int used;
};

void func(int n);

int func2(int c);

#endif
