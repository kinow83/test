/*
 * string_util.h
 *
 *  Created on: 2016. 10. 24.
 *      Author: root
 */

#ifndef OPENSSL_STRING_UTIL_H_
#define OPENSSL_STRING_UTIL_H_

#include <stdint.h>

uint8_t *append_string(uint8_t *dest, size_t dest_len, const uint8_t *add, int add_len);


#endif /* OPENSSL_STRING_UTIL_H_ */
