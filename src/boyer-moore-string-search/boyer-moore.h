/*
 * boyer-moore.h
 *
 *  Created on: 2017. 8. 11.
 *      Author: root
 */

#ifndef BOYER_MOORE_H_
#define BOYER_MOORE_H_

#include <stdint.h>

uint32_t boyer_moore (uint8_t *string, uint32_t stringlen, uint8_t *pat, uint32_t patlen, int *chars_compared);

#endif /* BOYER_MOORE_H_ */
