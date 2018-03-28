#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// https://www.mitchr.me/SS/exampleCode/AUPG/pcre_example.c.html

int main(int argc, char **argv)
{
	pcre *compiled;
	pcre_extra *pcre_extra;
	char *buf = malloc(20MB+1);
	

	buf[20MB] = 0;

	assert (readfile(argv[1], buf, 20MB) == 20MB);


}
