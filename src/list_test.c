/*
 * list_test.c
 *
 *  Created on: 2017. 6. 3.
 *      Author: root
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mini-clist.h"

struct Nation {
	int cityNo;
	char *cityName;
	struct list cities;
};


int main()
{
	struct list head;
	LIST_INIT(&head);


	struct Nation *n;
	n = (struct Nation*)malloc(sizeof(*n));
	n->cityNo = 1;
	n->cityName = strdup("KOREA");
	LIST_ADDQ(&head, &n->cities);

	n = (struct Nation*)malloc(sizeof(*n));
	n->cityNo = 2;
	n->cityName = strdup("USA");
	LIST_ADDQ(&head, &n->cities);

	n = (struct Nation*)malloc(sizeof(*n));
	n->cityNo = 3;
	n->cityName = strdup("CHINA");
	LIST_ADDQ(&head, &n->cities);

	n = (struct Nation*)malloc(sizeof(*n));
	n->cityNo = 4;
	n->cityName = strdup("JAPAN");
	LIST_ADDQ(&head, &n->cities);

	struct Nation *p, *pp;

	list_for_each_entry(p, &head, cities) {
		printf("%d: %s\n", p->cityNo, p->cityName);
	}
	printf("=======================\n");

	list_for_each_entry_safe(p, pp, &head, cities) {
		if (p->cityNo == 3) {
			LIST_DEL(&p->cities);
		}
	}

	list_for_each_entry(p, &head, cities) {
		printf("%d: %s\n", p->cityNo, p->cityName);
	}
}
