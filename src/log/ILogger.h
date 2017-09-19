/*
 * ILog.h
 *
 *  Created on: 2016. 8. 4.
 *      Author: root
 */

#ifndef ILOGGER_H_
#define ILOGGER_H_

#include <stdio.h>
#include <stdarg.h>

namespace kinow {

#ifndef byte
	typedef unsigned char byte;
#endif

#ifndef FIRE_LOG
	#define FIRE_LOG(msg, func) \
	do { \
		char buf[m_nBufSize]; \
		va_list ap; \
		va_start(ap, msg); \
		vsnprintf(buf, m_nBufSize, msg, ap); \
		func(buf); \
		va_end(ap); \
	} while (0)
#endif

#ifndef FIRE_LOG_HEX
	#define FIRE_LOG_HEX(hex, len, msg, func) \
	do { \
		char buf[m_nBufSize]; \
		va_list ap; \
		va_start(ap, msg); \
		vsnprintf(buf, m_nBufSize, msg, ap); \
		func(hex, len, buf); \
		va_end(ap); \
	} while (0)
#endif

#ifndef L_LV
typedef enum {
	L_CRI = 1,
	L_ERR = 2,
	L_INF = 4,
	L_DBG = 8,
	L_ALL = 15
} L_LV;
#endif

class ILogger {

public:
	void level(L_LV lvl) { m_lvl = lvl; }
	void c(const char* msg, ...) { if (m_lvl & L_CRI) FIRE_LOG(msg, fireCri); }
	void C(const char* msg, ...) { if (m_lvl & L_CRI) FIRE_LOG(msg, fireCRI); }
	void e(const char* msg, ...) { if (m_lvl & L_ERR) FIRE_LOG(msg, fireErr); }
	void E(const char* msg, ...) { if (m_lvl & L_ERR) FIRE_LOG(msg, fireERR); }
	void i(const char* msg, ...) { if (m_lvl & L_INF) FIRE_LOG(msg, fireInf); }
	void I(const char* msg, ...) { if (m_lvl & L_INF) FIRE_LOG(msg, fireINF); }
	void d(const char* msg, ...) { if (m_lvl & L_DBG) FIRE_LOG(msg, fireDbg); }
	void D(const char* msg, ...) { if (m_lvl & L_DBG) FIRE_LOG(msg, fireDBG); }
	void chex(byte* hex, int len, const char* msg, ...) {
		if (m_lvl & L_CRI) FIRE_LOG_HEX(hex, len, msg, fireCriHex);
	}
	void ehex(byte* hex, int len, const char* msg, ...) {
		if (m_lvl & L_ERR) FIRE_LOG_HEX(hex, len, msg, fireErrHex);
	}
	void ihex(byte* hex, int len, const char* msg, ...) {
		if (m_lvl & L_INF) FIRE_LOG_HEX(hex, len, msg, fireInfHex);
	}
	void dhex(byte* hex, int len, const char* msg, ...) {
		if (m_lvl & L_DBG) FIRE_LOG_HEX(hex, len, msg, fireDbgHex);
	}

protected:
	void fireNextCri(const char* msg) { if (m_next) m_next->fireCri(msg); }
	void fireNextCRI(const char* msg) { if (m_next) m_next->fireCRI(msg); }
	void fireNextErr(const char* msg) { if (m_next) m_next->fireErr(msg); }
	void fireNextERR(const char* msg) { if (m_next) m_next->fireERR(msg); }
	void fireNextInf(const char* msg) { if (m_next) m_next->fireInf(msg); }
	void fireNextINF(const char* msg) { if (m_next) m_next->fireINF(msg); }
	void fireNextDbg(const char* msg) { if (m_next) m_next->fireDbg(msg); }
	void fireNextDBG(const char* msg) { if (m_next) m_next->fireDBG(msg); }
	void fireNextCriHex(byte* hex, int len, const char* msg) { if (m_next) m_next->fireCriHex(hex, len, msg); }
	void fireNextErrHex(byte* hex, int len, const char* msg) { if (m_next) m_next->fireErrHex(hex, len, msg); }
	void fireNextInfHex(byte* hex, int len, const char* msg) { if (m_next) m_next->fireInfHex(hex, len, msg); }
	void fireNextDbgHex(byte* hex, int len, const char* msg) { if (m_next) m_next->fireDbgHex(hex, len, msg); }

protected:
	virtual void fireCri(const char* msg) = 0;
	virtual void fireCRI(const char* msg) = 0;
	virtual void fireErr(const char* msg) = 0;
	virtual void fireERR(const char* msg) = 0;
	virtual void fireInf(const char* msg) = 0;
	virtual void fireINF(const char* msg) = 0;
	virtual void fireDbg(const char* msg) = 0;
	virtual void fireDBG(const char* msg) = 0;
	virtual void fireCriHex(byte* hex, int len, const char* msg) = 0;
	virtual void fireErrHex(byte* hex, int len, const char* msg) = 0;
	virtual void fireInfHex(byte* hex, int len, const char* msg) = 0;
	virtual void fireDbgHex(byte* hex, int len, const char* msg) = 0;

public:
	const static size_t DEFAULT_BUF_SIZE = 1024;
	ILogger(L_LV lvl) :
		m_lvl(lvl),
		m_next(NULL),
		m_nBufSize(DEFAULT_BUF_SIZE)
	{}
	ILogger(L_LV lvl, ILogger* next) :
		m_lvl(lvl),
		m_next(next),
		m_nBufSize(DEFAULT_BUF_SIZE)
	{}

private:
	ILogger* m_next;
	int m_nBufSize;
	L_LV m_lvl;
};

} /* namespace kinow */

#endif /* ILOGGER_H_ */
