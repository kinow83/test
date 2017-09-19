/*
 * endian.h
 *
 *  Created on: 2016. 10. 24.
 *      Author: root
 */

#ifndef STRINGS_TEST_ENDIAN_H_
#define STRINGS_TEST_ENDIAN_H_

#ifndef IS_BIG_ENDIAN
 #define IS_BIG_ENDIAN (*(unsigned short *)"\0\xff" < 0x100)
#endif

#endif /* STRINGS_TEST_ENDIAN_H_ */
