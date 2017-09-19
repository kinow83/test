/*
 * Error.h
 *
 *  Created on: 2016. 8. 12.
 *      Author: root
 */

#ifndef ERROR_H_
#define ERROR_H_

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

namespace kinow {

class Error {
public:
	Error() { }
	virtual ~Error() { }

public:
	const char* cause() { return m_szBuf; }
	void success() { m_szBuf[0] = '\0'; }
	void failure(const char* msg, ...) {
		va_list ap;
		va_start(ap, msg);
		vsnprintf(m_szBuf, bufSize, msg, ap);
		va_end(ap);
	}

private:
	static const int bufSize = 1024;
	char m_szBuf[bufSize];
};

} /* namespace kinow */

#endif /* ERROR_H_ */
