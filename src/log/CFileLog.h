/*
 * Log.h
 *
 *  Created on: 2016. 8. 2.
 *      Author: root
 */

#ifndef CFILELOG_H_
#define CFILELOG_H_

#include <string.h>
#include <time.h>
#include "ILogger.h"

namespace kinow {

class CFileLog : public ILogger {
public:
	CFileLog(L_LV lvl, const char* dir, const char* filename) :
		ILogger(lvl),
		m_szFileDir(strdup(dir)),
		m_szFileName(strdup(filename))
	{}
	CFileLog(L_LV lvl, const char* dir, const char* filename, ILogger* next) :
		ILogger(lvl, next),
		m_szFileDir(strdup(dir)),
		m_szFileName(strdup(filename))
	{}
	virtual ~CFileLog();


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
	void fireLog(L_LV lvl, const char* msg);
	void fireLogHex(L_LV lvl, byte* hex, int len, const char* msg);
	bool makeDir(const char* dir);
	void logrotate(const char* filePath, struct tm& stNow);
	FILE* getCacheFile(struct tm& stNow);

private:
	char* m_szFileDir;
	char* m_szFileName;
	FILE* m_cacheFp;
	struct tm m_cacheTm;

	const static int KB = 1024;
	const static int MB = 1024 * KB;
	const static int GB = 1024 * MB;
};

} /* namespace kinow */

#endif /* CFILELOG_H_ */
