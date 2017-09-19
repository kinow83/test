/*
 * StringTok.h
 *
 *  Created on: 2016. 10. 21.
 *      Author: root
 */

#ifndef STRINGS_STRINGTOK_H_
#define STRINGS_STRINGTOK_H_

#include <vector>
#include <string>

namespace kinow {

class StringTok {
public:
	StringTok(const char* text, const char* delim, bool trim);
	virtual ~StringTok();
	const char* operator[](int index) const;
	int size();

private:
	void tok();

private:
	char *_orgText;
	char *_delim;
	std::vector<std::string*> _tokVec;
	bool _trim;
};

} /* namespace kinow */

#endif /* STRINGS_STRINGTOK_H_ */
