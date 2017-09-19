/*
 * string_util.c
 *
 *  Created on: 2016. 10. 24.
 *      Author: root
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "string_util.h"


uint8_t *append_string(uint8_t *dest, size_t dest_len, const uint8_t *add, int add_len)
{
	uint8_t * res_str = NULL;
	int i;

	if ( (dest == NULL) || (dest_len == 0) )
	{
		res_str = (uint8_t *)malloc(sizeof(uint8_t) * (add_len+1));
		for (i=0; i<add_len; i++)
			res_str[i] = add[i];
		res_str[i] = 0;
	}
	else
	{
		res_str = (uint8_t *)malloc(sizeof(uint8_t) * (dest_len + add_len + 1));
		for (i=0; i<dest_len; i++)
			res_str[i] = dest[i];
		for (i=0; i<add_len; i++)
			res_str[i + dest_len] = add[i];
		res_str[i + dest_len] = 0;
	}
	free(dest);

	return res_str;
}


