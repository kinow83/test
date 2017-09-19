/*
 * CConsoleLog.h
 *
 *  Created on: 2016. 8. 4.
 *      Author: root
 */

#ifndef CCONSOLELOG_H_
#define CCONSOLELOG_H_

#include "ILogger.h"

namespace kinow {

class CConsoleLog : public ILogger {
public:
	CConsoleLog(L_LV lvl) :
		ILogger(lvl)
	{}
	CConsoleLog(L_LV lvl, ILogger* next) :
		ILogger(lvl, next)
	{}
	virtual ~CConsoleLog() {}

protected:
	virtual void fireCri(const char* msg);
	virtual void fireCRI(const char* msg);
	virtual void fireErr(const char* msg);
	virtual void fireERR(const char* msg);
	virtual void fireInf(const char* msg);
	virtual void fireINF(const char* msg);
	virtual void fireDbg(const char* msg);
	virtual void fireDBG(const char* msg);
	virtual void fireCriHex(byte* hex, int len, const char* msg);
	virtual void fireErrHex(byte* hex, int len, const char* msg);
	virtual void fireInfHex(byte* hex, int len, const char* msg);
	virtual void fireDbgHex(byte* hex, int len, const char* msg);

private:
	void fireLog(L_LV lvl, const char* msg, const char* color, bool bold);
	void fireLogHex(L_LV lvl, byte* hex, int len, const char* msg, const char* color, bool bold);
};

} /* namespace kinow */

#endif /* CCONSOLELOG_H_ */
