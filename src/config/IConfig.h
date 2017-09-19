/*
 * IConfig.h
 *
 *  Created on: 2016. 8. 5.
 *      Author: root
 */

#ifndef ICONFIG_H_
#define ICONFIG_H_

#include <stdlib.h>
#include <strings.h>
#include <string.h>

namespace kinow {

class IConfig {

public:
	virtual void show() = 0;
	virtual bool load() = 0;
	virtual bool load(const char* key) = 0;

public:
	virtual const char* getString(const char* key, const char* defval) = 0;

	const int getInt(const char* key, const char* defval) {
		return atoi((const char*)getString(key, defval));
	}
	const long getLong(const char* key, const char* defval) {
		return atol((const char*)getString(key, defval));
	}
	const long long getLongLong(const char* key, const char* defval) {
		return atoll((const char*)getString(key, defval));
	}
	const bool getBool(const char* key, const char* defval) {
		const char *value = getString(key, defval);
		if (!strcasecmp("TRUE", value) || !strcasecmp("YES", value))
			return true;
		return false;
	}

public:
	virtual const char* getString(const char * section, const char* key, const char* defval) = 0;

	const int getInt(const char * section, const char* key, const char* defval) {
		return atoi((const char*)getString(section, key, defval));
	}
	const long getLong(const char * section, const char* key, const char* defval) {
		return atol((const char*)getString(section, key, defval));
	}
	const long long getLongLong(const char * section, const char* key, const char* defval) {
		return atoll((const char*)getString(section, key, defval));
	}
	const bool getBool(const char * section, const char* key, const char* defval) {
		const char *value = getString(section, key, defval);
		if (!strcasecmp("TRUE", value) || !strcasecmp("YES", value))
			return true;
		return false;
	}
};

} /* namespace kinow */

#endif /* ICONFIG_H_ */
