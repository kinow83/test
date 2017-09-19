/*
 * ConvertHelper.h
 *
 *  Created on: 2016. 10. 24.
 *      Author: root
 */

#ifndef STRINGS_CONVERTHELPER_H_
#define STRINGS_CONVERTHELPER_H_

#include <stdint.h>

namespace kinow {

class ConvertHelper {

public:
	static bool string2mac(const char* str, unsigned char* bytes6);
	static bool string2ipv4(const char* str, uint32_t* ipv4);
	static const char* mac2string(char* str, size_t strlen, unsigned char* bytes6, bool uppercase);
	static const char* ipv42string(char* str, size_t strlen, uint32_t ipv4);

private:
	ConvertHelper() {}
	virtual ~ConvertHelper() {}
};

} /* namespace kinow */

#endif /* STRINGS_CONVERHELPER_H_ */
