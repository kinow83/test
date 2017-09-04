/*
 * fmemopen.c
 *
 *  Created on: 2017. 8. 25.
 *      Author: root
 */


#include <stdio.h>
#include <string.h>


int main()
{
	static char buffer[] = "kaka hello!";
	int ch;
	FILE *stream;
	char buf[1024];

	stream = fmemopen(buffer, strlen(buffer), "r");
	while ((ch = fgetc(stream)) != EOF) {
		printf("fgetc: %c\n", ch);
	}

	fseek(stream, SEEK_SET, 0);
	fgets(buf, sizeof(buf), stream);
	printf("fgets: %s\n", buf);

	fseek(stream, SEEK_SET, 0);
	fprintf(stream, "%s", buf);
	printf("fprintf: %s\n", buf);


	fclose(stream);

	return 0;
}
