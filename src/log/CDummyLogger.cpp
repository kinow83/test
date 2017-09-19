/*
 * CDummyLogger.cpp
 *
 *  Created on: 2016. 8. 4.
 *      Author: root
 */

#include <stdio.h>
#include "ILogger.h"

namespace kinow {

class CDummyLogger : public ILogger {
public:
	CDummyLogger(L_LV lvl) : ILogger(lvl) { }
	CDummyLogger(L_LV lvl, ILogger* log) : ILogger(lvl, log) { }
	~CDummyLogger();

protected:
	virtual void fireCri(const char* msg) {
		printf("%s", msg);
//		fireNextCri(msg);
	}
	virtual void fireCRI(const char* msg) {

	}
	virtual void fireErr(const char* msg) {

	}
	virtual void fireERR(const char* msg) {

	}
	virtual void fireInf(const char* msg) {

	}
	virtual void fireINF(const char* msg) {

	}
	virtual void fireDbg(const char* msg) {

	}
	virtual void fireDBG(const char* msg) {

	}
	virtual void fireCriHex(byte* hex, int len, const char* msg) {

	}
	virtual void fireErrHex(byte* hex, int len, const char* msg) {

	}
	virtual void fireInfHex(byte* hex, int len, const char* msg) {

	}
	virtual void fireDbgHex(byte* hex, int len, const char* msg) {

	}
};

} /* namespace kinow */

using namespace kinow;

int main() {
	L_LV lv = L_CRI;
	ILogger* logger1 = new CDummyLogger(lv);
	ILogger* logger2 = new CDummyLogger(lv, logger1);

	logger2->c("kkkkkkkkkkkkkkk :%d :%s\n", 10, "hhhogogogogogog");
}
