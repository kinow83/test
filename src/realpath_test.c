/*
 * realpath_test.c
 *
 *  Created on: 2017. 6. 7.
 *      Author: root
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main(int arc, char **argv)
{
	char *ppath;
	char path[PATH_MAX];
	ppath = realpath(argv[1], path);
	printf("%s\n", path);
	printf("%d\n", path == ppath);
	printf("%s\n", ppath);
}
