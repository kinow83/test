#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _ChainFuncPointList ChainFuncPointList;

typedef struct _Context
{
	ChainFuncPointList *chain_func_list;
} Context;

typedef struct _ChainFuncPointList
{
	char *name;
	int (*handler)(Context *, ChainFuncPointList *, void *);
	struct _ChainFuncPointList *next;
} ChainFuncPointList;

void addChainFuncPoint(Context *context, const char *name, int (*handler)(Context *, ChainFuncPointList *, void *))
{
	ChainFuncPointList *idx = context->chain_func_list;

	if (idx == NULL) {
		idx = (ChainFuncPointList *)calloc(1, sizeof(ChainFuncPointList));
		idx->name = strdup(name);
		idx->handler = handler;
		context->chain_func_list = idx;
//		printf("new chain: %s\n", name);
	} else {
		while (idx->next)
			idx = idx->next;

		idx->next = (ChainFuncPointList *)calloc(1, sizeof(ChainFuncPointList));
		idx->next->name = strdup(name);
		idx->next->handler = handler;
//		printf("add chain: %s\n", name);
	}
}

int numChainFuncPoint(Context *context)
{
	int num = 0;
	ChainFuncPointList *idx = context->chain_func_list;

	while (idx) {
//		printf(" ==> %s\n", idx->name);
		num++;
		idx = idx->next;
	}
	return num;
}

void freeChainFuncPoint(ChainFuncPointList *chain)
{
	ChainFuncPointList *tmp = chain;
	while (chain) {
		tmp = chain->next;
		free(chain->name);
		free(chain);
		chain = tmp;
	}
}

int helloChain(Context *context, ChainFuncPointList *chain, void *data)
{
	printf("hello\n");
	return chain->next->handler(context, chain->next, data);
}

int kakaChain(Context *context, ChainFuncPointList *chain, void *data)
{
	printf("kaka\n");
	return chain->next->handler(context, chain->next, data);
}

int endChain(Context *context, ChainFuncPointList *chain, void *data)
{
	printf("end\n");
	return 1;
}

int doChain(Context *context, void *data)
{
	int rs = 1;
	ChainFuncPointList *idx = context->chain_func_list;

	if (idx) {
		rs = idx->handler(context, idx, data);
	}

	return rs;
}

int main()
{
	Context *context = (Context*)calloc(1, sizeof(Context));

	addChainFuncPoint(context, "hello_func", helloChain);
	addChainFuncPoint(context, "kaka_func",  kakaChain);
	addChainFuncPoint(context, "end_func",   endChain);
	
	printf("num: %d\n", numChainFuncPoint(context));

	doChain(context, NULL);

	freeChainFuncPoint(context->chain_func_list);

	free(context);

	return 0;
}
