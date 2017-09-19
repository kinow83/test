/*
 * Log.cpp
 *
 *  Created on: 2016. 8. 2.
 *      Author: root
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include "CFileLog.h"

namespace kinow {

void CFileLog::fireCri(const char* msg) {
	fireLog(L_CRI, msg);
	fireNextCri(msg);
}
void CFileLog::fireCRI(const char* msg) {
	fireLog(L_CRI, msg);
	fireNextCRI(msg);
}
void CFileLog::fireErr(const char* msg) {
	fireLog(L_ERR, msg);
	fireNextErr(msg);
}
void CFileLog::fireERR(const char* msg) {
	fireLog(L_ERR, msg);
	fireNextERR(msg);
}
void CFileLog::fireInf(const char* msg) {
	fireLog(L_INF, msg);
	fireNextInf(msg);
}
void CFileLog::fireINF(const char* msg) {
	fireLog(L_INF, msg);
	fireNextINF(msg);
}
void CFileLog::fireDbg(const char* msg) {
	fireLog(L_DBG, msg);
	fireNextDbg(msg);
}
void CFileLog::fireDBG(const char* msg) {
	fireLog(L_DBG, msg);
	fireNextDBG(msg);
}
void CFileLog::fireCriHex(byte* hex, int len, const char* msg) {
	fireLogHex(L_CRI, hex, len, msg);
	fireNextCriHex(hex, len, msg);
}
void CFileLog::fireErrHex(byte* hex, int len, const char* msg) {
	fireLogHex(L_ERR, hex, len, msg);
	fireNextErrHex(hex, len, msg);
}
void CFileLog::fireInfHex(byte* hex, int len, const char* msg) {
	fireLogHex(L_INF, hex, len, msg);
	fireNextInfHex(hex, len, msg);
}
void CFileLog::fireDbgHex(byte* hex, int len, const char* msg) {
	fireLogHex(L_DBG, hex, len, msg);
	fireNextDbgHex(hex, len, msg);
}

CFileLog::~CFileLog() {
	if (m_szFileDir) free(m_szFileDir);
	if (m_szFileName) free(m_szFileName);
	if (m_cacheFp) fclose(m_cacheFp);
}

bool CFileLog::makeDir(const char* dir) {
	DIR *pDir = NULL;

	pDir = opendir(dir);
	if (pDir) {
		closedir(pDir);
		return true;
	}
	closedir(pDir);
	if (errno == ENOENT) {
		if (mkdir(dir, 0655)) {
			return false;
		}
	}
	return true;
}

void CFileLog::logrotate(const char* filePath, struct tm& stNow) {
	struct stat stInfo;
	DIR *pDir = NULL;
	struct dirent *pDirEnt = NULL;
	char sameFile[256] = {0, };
	char newFile[256] = {0, };
	char cmd[256] = {0, };
	int nFileCount = 0;

	if (stat(filePath, &stInfo) == 0) {
		// file size is over.
		if (stInfo.st_size > (long long)(1*GB)) {
			pDir = opendir(m_szFileDir);
			if (pDir == NULL) {
				if (!makeDir(m_szFileDir)) {
					fprintf(stderr, "ERROR: '%s' can't open directory - %s\n", m_szFileDir, strerror(errno));
					return;
				}
			}
			snprintf(sameFile, sizeof(sameFile), "%s-%04d%02d%02d-",
					m_szFileName, stNow.tm_year+1900, stNow.tm_mon+1, stNow.tm_mday);
			while ((pDirEnt = readdir(pDir)) != 0) {
				if (strstr(pDirEnt->d_name, sameFile)) {
					nFileCount++;
				}
			}
			(void)closedir(pDir);

			// TODO: max log file is 999. (%03d)
			snprintf(newFile, sizeof(newFile), "%s/%s-%04d%02d%02d-%03d.log",
					m_szFileDir, m_szFileName, stNow.tm_year+1900, stNow.tm_mon+1, stNow.tm_mday, nFileCount);
			if (rename(filePath, newFile) == -1) {
				fprintf(stderr, "ERROR: can't rename %s to %s - %s\n", filePath, newFile, strerror(errno));
				return;
			}
		}
	}
}

FILE* CFileLog::getCacheFile(struct tm& stNow) {
	if ((m_cacheTm.tm_mon != stNow.tm_mon) && ((m_cacheTm.tm_mday != stNow.tm_mday))) {
		m_cacheFp = NULL;
	}
	return m_cacheFp;
}

void CFileLog::fireLog(L_LV lvl, const char* msg) {
	char filePath[256];
	FILE *fp = NULL;
	struct tm stNow;
	char szTime[256] = {0,};
	const char* pLvl;
	time_t now = time(NULL);

	localtime_r(&now, &stNow);
	strftime(szTime, sizeof(szTime), "%H:%M:%S", &stNow);

	snprintf(filePath, sizeof(filePath), "%s/%s-%04d%02d%02d.log",
			m_szFileDir, m_szFileName, stNow.tm_year+1900, stNow.tm_mon+1, stNow.tm_mday);

	logrotate(filePath, stNow);

	fp = getCacheFile(stNow);
	if (fp == NULL) {
		fp = fopen(filePath, "a+");
		if (fp == NULL) {
			fprintf(stderr, "ERROR: %s open error - %s\n", filePath, strerror(errno));
			return;
		}
	}

	switch (lvl) {
	case L_CRI:
		pLvl = "CRI";
		break;
	case L_ERR:
		pLvl = "ERR";
		break;
	case L_INF:
		pLvl = "INF";
		break;
	case L_DBG:
		pLvl = "DBG";
		break;
	}
	fprintf(fp, "%s :%s: %s\n", szTime, pLvl, msg);
	if (fp) {
		fclose(fp);
	}
}

void CFileLog::fireLogHex(L_LV lvl, byte* hex, int len, const char* msg) {
	char filePath[256];
	FILE *fp = NULL;
	struct tm stNow;
	char szTime[256] = {0,};
	const char* pLvl;
	int i;
	time_t now = time(NULL);

	localtime_r(&now, &stNow);
	strftime(szTime, sizeof(szTime), "%H:%M:%S", &stNow);

	snprintf(filePath, sizeof(filePath), "%s/%s-%04d%02d%02d.log",
			m_szFileDir, m_szFileName, stNow.tm_year+1900, stNow.tm_mon+1, stNow.tm_mday);

	logrotate(filePath, stNow);

	fp = fopen(filePath, "a+");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: %s open error - %s\n", filePath, strerror(errno));
		return;
	}

	switch (lvl) {
	case L_CRI:
		pLvl = "CRI";
		break;
	case L_ERR:
		pLvl = "ERR";
		break;
	case L_INF:
		pLvl = "INF";
		break;
	case L_DBG:
		pLvl = "DBG";
		break;
	}
	fprintf(fp, "%s :%s: %s (len: %d)\n", szTime, pLvl, msg, len);
	for (i=1; i<=len; i++) {
		fprintf(fp, "%02x ", hex[i-1]);
		if (i%8 == 0) fprintf(fp, " ");
		if (i%16 == 0) fprintf(fp, "\n");
	}
	if (fp) {
		fclose(fp);
	}
}

} /* namespace kinow */

#ifdef TEST
using namespace kinow;

int main() {
	const char* desc = "kaka kinow 123456 GoGo ~~ !!!";
	byte hex[64];

	for (int i=0; i<sizeof(hex); i++) {
		hex[i] = i;
	}

	ILogger *l = new CFileLog(L_ALL, ".", "kaka");

	l->c(desc);
	l->C(desc);
	l->e(desc);
	l->E(desc);
	l->i(desc);
	l->I(desc);
	l->d(desc);
	l->D(desc);

	l->chex(hex, sizeof(hex), desc);
	l->ehex(hex, sizeof(hex), desc);
	l->ihex(hex, sizeof(hex), desc);
	l->dhex(hex, sizeof(hex), desc);
}
#endif
