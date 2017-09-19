/*
 * CFileConfig.h
 *
 *  Created on: 2016. 8. 5.
 *      Author: root
 */

#ifndef CFILECONFIG_H_
#define CFILECONFIG_H_

#include <map>
#include <string>
#include <vector>
#include <string.h>
#include "IConfig.h"

namespace kinow {

class CFileConfig : public IConfig {

public:
	CFileConfig(const char* filename);
	virtual ~CFileConfig();

public:
	void show();
	bool load();
	bool load(const char* key);
	const char* getString(const char* key, const char* defval);
	const char* getString(const char* section, const char* key, const char* defval);

protected:
	char* trim(char* origing, const char* delim);
	char* leftTrim(char *origin, const char *delim);
	char* rightTrim(char* origin, const char* delim);
	bool load(const char* filename, const char* key);

private:
	typedef enum {
		val_none = 0,
		val_open = 1,
		val_close = 2,
	} VAL_OPEN_T;
	typedef std::map<std::string, std::string> inner_map_t;
	char* m_filename;
	std::map<std::string, inner_map_t> m_sectionConfig;
	std::map<std::string, std::string> m_generalConfig;

private:
	std::string parseValue(const char* value, int value_size, int line);
	std::vector<std::string> sectionList();
};

} /* namespace kinow */

#endif /* CFILECONFIG_H_ */
