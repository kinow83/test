/*
 * StringTok.cpp
 *
 *  Created on: 2016. 10. 21.
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StringTok.h"

namespace kinow {

#define TRIM_SPACE " \t\n\v"

/**
 * find() : 주어진 부분문자열이 처음으로 나타나는 곳을 찾는다
 * find_first_of()     : 주어진 문자가 처음으로 나타나는 위치를 찾는다
 * find_first_not_of() : 주어진 문자가 아닌 첫 문자가 처음으로 나타나는 위치를 찾는다
 * find_last_of()      : 주어진 문자가 마지막으로 나타나는 위치를 찾는다
 * find_last_not_of()  : 주어진 문자가 아닌 문자를 찾는다.
 * rfind() : find()와 같으나, 찾는 방향이 반대이다. (뒤쪽부터 찾는다)
 */

inline static std::string& rtrim(
		std::string& str,
		const std::string& drop = TRIM_SPACE)
{
	return str.erase( str.find_last_not_of(drop) + 1 );
}

inline static std::string& ltrim(
		std::string& str,
		const std::string& drop = TRIM_SPACE)
{
	return str.erase( 0, str.find_first_not_of(drop) );
}

inline static std::string& trim(
		std::string& str,
		const std::string& drop = TRIM_SPACE)
{
	return ltrim(rtrim(str, drop), drop);
}


StringTok::StringTok(const char* text, const char* delim, bool trim) {
	_orgText = strdup(text);
	_delim = strdup(delim);
	_trim = trim;

	// do token
	tok();
}

StringTok::~StringTok() {
	int i = 0;

	if (_orgText) free(_orgText);
	if (_delim) free(_delim);

	for (i=0; i<_tokVec.size(); i++) {
		if (_tokVec[i]) {
			delete _tokVec[i];
			_tokVec[i] = NULL;
		}
	}
	_tokVec.clear();

}
void StringTok::tok() {
	char *token = NULL;
	char *last = NULL;
	int i = 0;
	std::string str;

	token = strtok_r(_orgText, _delim, &last);
	while (token) {
		str = std::string(token);

		if (_trim) str = trim(str);

		_tokVec.push_back( new std::string(str) );

		token = strtok_r(NULL, _delim, &last);
	}
}

const char* StringTok::operator[](int index) const {
	if (index >= _tokVec.size()) {
		return NULL;
	}
	return (_tokVec[index]->c_str());
}

int StringTok::size() {
	return _tokVec.size();
}

} /* namespace kinow */

/*
using namespace kinow;
int main(int argc, char **argv) {
	int i=0;
	StringTok *s;
	s = new StringTok("kinow, wowman,    fff,,,  , ", ",", true);

	while ((*s)[i] != NULL) {
		printf("tok[%d] = %s\n", i, (*s)[i]);
		i++;
	}

	delete s;
}
*/
