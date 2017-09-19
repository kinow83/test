/*
 * CFileConfig.cpp
 *
 *  Created on: 2016. 8. 5.
 *      Author: root
 */

#include "CFileConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

namespace kinow {

static const char* INCLUDE_SYMBOL = "$include";

CFileConfig::CFileConfig(const char* filename) :
		m_filename(strdup(filename)) {
//	load();
}

CFileConfig::~CFileConfig() {
	if (m_filename) free(m_filename);
}

std::vector<std::string> CFileConfig::sectionList() {
	std::map<std::string, inner_map_t>::iterator it;
	std::vector<std::string> list;

	for (	it = m_sectionConfig.begin();
				it != m_sectionConfig.end();
				++it) {
		list.push_back(it->first);
	}
	return list;
}

bool CFileConfig::load() {
	FILE *fp = NULL;
	char szLine[1024] = {0, };
	char *pKey = NULL;
	char *pVal = NULL;
	char *pLine = NULL;
	char *pTokLast = NULL;
	char *pInclude = NULL;
	std::map<std::string, std::string>::iterator it;
	char *pSectionTok = NULL;
	char *pSection = NULL;
	std::map<std::string, std::string>* curConfig = &m_generalConfig;
	int nLineCount = 0;
	char szVal[256] = {0, };
	int nValLen = 0;

	fp = fopen(m_filename, "r");
	if (!fp) {
		fprintf(stderr, "file not found - %s\n", strerror(errno));
		return false;
	}

NEXT_LINE:
	while (fgets(szLine, sizeof(szLine), fp)) {
		if (feof(fp)) {
			break;
		}
		nLineCount++;
		pLine = szLine;
		if (*pLine == '#' || *pLine == '*') {
			continue;
		}

		pLine = trim(pLine, " \t\n");

		if (!strncmp(INCLUDE_SYMBOL, pLine, strlen(INCLUDE_SYMBOL))) {

			pInclude = strtok_r(pLine + strlen(INCLUDE_SYMBOL), " \t\n", &pTokLast);
			if (pInclude == NULL) {
				fprintf(stderr, "%s ???\n", INCLUDE_SYMBOL);
				fclose(fp);
				return false;
			}
			// recursive call 'road'
			load(pInclude);

		} else if ((pSectionTok = strchr(pLine, '{'))) {
			if (pSection) {
				fprintf(stderr, "[file:%s] missing '}' section. line: %d\n", m_filename, nLineCount);
				exit(1);
			}
			pSection = (char*) malloc(pSectionTok - pLine + 1);
			memset(pSection, 0, pSectionTok - pLine + 1);
			strncpy(pSection, pLine, pSectionTok - pLine);
			pSection = trim(pSection, " \t");

			// section config map
			curConfig = &m_sectionConfig[std::string(pSection)];

		} else if ((pSectionTok = strchr(pLine, '}'))) {
			if (!pSection) {
				fprintf(stderr, "[file:%s] missing '{' section. line: %d\n", m_filename, nLineCount);
				exit(1);
			}
			// general config map
			curConfig = &m_generalConfig;
			free(pSection);
			pSection = NULL;

		} else {
			pKey = strtok_r(pLine, " \t\n=", &pTokLast);
			if (pKey == NULL) {
				continue;
			}

			nValLen = 0;
			memset(szVal, 0, sizeof(szVal));
			pVal = strtok_r(NULL, " \t\n=\x0D\x0A", &pTokLast);
			if (pVal == NULL) {
				goto NEXT_LINE;
			}
			nValLen += snprintf(szVal + nValLen, sizeof(szVal) - nValLen, "%s", pVal);

			while ((pVal = strtok_r(NULL, " \t\n\x0D\x0A", &pTokLast))) {
				nValLen += snprintf(szVal + nValLen, sizeof(szVal) - nValLen, " %s", pVal);
			};

//			lowercase(key);
			it = curConfig->find(std::string(pKey));
			if (it != curConfig->end()) {
				fprintf(stderr, "[file:%s] key:'%s' = %s is predefined. line: %d\n",
						m_filename, pKey, it->second.c_str(), nLineCount);
				exit(1);
			}

			(*curConfig)[std::string(pKey)] = parseValue(szVal, strlen(szVal), nLineCount);
		}
	}

	if (pSection) {
		fprintf(stderr, "[file:%s] missing '}' section. line: %d\n", m_filename, nLineCount);
		exit(1);
	}

	fclose(fp);
	return true;
}

std::string CFileConfig::parseValue(const char* value, int value_size, int line) {
	int i = 0;
	char ch = 0;
	VAL_OPEN_T vtype = val_none;
	char szContext[512] = {0, };
	int ContextPos = 0;
	char szVal[128] = {0, };
	size_t valPos = 0;
	std::map<std::string, std::string>::iterator it;

	for (i=0; i<value_size; i++) {
		ch = value[i];

		if (ch == '[') {
			vtype = val_open;
			memset(szVal, 0, sizeof(szVal));
			valPos = 0;
		}
		else if (ch == ']') {
			vtype = val_close;
			it = m_generalConfig.find(szVal);
			if (it == m_generalConfig.end()) {
				fprintf(stderr, "[file:%s] not found '%s' variable. line: %d\n", m_filename, szVal, line);
				exit(1);
			}
			if (it->second.length() + ContextPos > sizeof(szContext) - 1) {
				fprintf(stderr, "[file:%s] '%s' value is too big. line: %d\n", m_filename, szContext, line);
				exit(1);
			}
			strcpy(szContext + ContextPos, it->second.c_str());
			ContextPos += it->second.length();
		}
		else {
			if (vtype == val_open) {
				if (valPos > sizeof(szVal) - 1) {
					fprintf(stderr, "[file:%s] variable is too big. line: %d\n", m_filename, line);
					exit(1);
				}
				szVal[valPos++] = ch;
			} else {
				if (ContextPos > (int)sizeof(szContext) - 1) {
					fprintf(stderr, "[file:%s] value is too big. line: %d\n", m_filename, line);
					exit(1);
				}
				szContext[ContextPos++] = ch;
			}
		}
	}
	return szContext;
}

bool CFileConfig::load(const char* key) {
	return load(m_filename, key);
}

bool CFileConfig::load(const char* filename, const char* key) {
	FILE *fp = NULL;
	char szLine[1024] = {0, };
	char *pKey = NULL;
	char *pVal = NULL;
	char *pLine = NULL;
	char *pLastTok = NULL;
	char *pInclude = NULL;

	fp = fopen(m_filename, "r");
	if (!fp) {
		fprintf(stderr, "file not found - %s\n", m_filename);
		return false;
	}

	while (fgets(szLine, sizeof(szLine), fp)) {
		if (feof(fp)) {
			break;
		}
		pLine = szLine;
		if (*pLine == '#' || *pLine == '*') {
			continue;
		}
		if (!strncmp(INCLUDE_SYMBOL, pLine, strlen(INCLUDE_SYMBOL))) {
			pInclude = strtok_r(pLine + strlen(INCLUDE_SYMBOL)," \t\n", &pLastTok);
			if (pInclude == NULL) {
				fprintf(stderr, "%s ???\n", INCLUDE_SYMBOL);
				fclose(fp);
				return false;
			}
			load(pInclude, key);
		}
		else {
			pKey = strtok_r(pLine," \t\n=", &pLastTok);
			if (pKey == NULL) {
				continue;
			}
			if (strcmp(pKey, key)) {
				continue;
			}

			pVal = strtok_r( NULL," \t\n=\x0D\x0A", &pLastTok);
			if (pVal == NULL) {
				continue;
			}

//			lowercase(pKey);
			m_generalConfig[std::string(pKey)] = std::string(pVal);
		}
	}
	fclose(fp);
	return true;
}

void CFileConfig::show() {
	std::map<std::string, std::string> *map2 = NULL;
	std::map<std::string, std::string>::iterator it1;
	std::map<std::string, inner_map_t>::iterator it2;

	fprintf(stdout, "filename: %s\n", m_filename);
	for (	it1 = m_generalConfig.begin();
			it1 != m_generalConfig.end();
			++it1) {
		fprintf(stdout, "%s = %s\n", it1->first.c_str(), it1->second.c_str());
	}

	for (	it2 = m_sectionConfig.begin();
			it2 != m_sectionConfig.end();
			++it2) {
		fprintf(stdout, "%s {\n", it2->first.c_str());
		map2 = &it2->second;
		for (it1 = map2->begin(); it1 != map2->end(); ++it1) {
			fprintf(stdout, "\t%s = %s\n", it1->first.c_str(), it1->second.c_str());
		}
		fprintf(stdout, "}\n");
	}
}

const char* CFileConfig::getString(const char* key, const char* defval) {
	std::map<std::string, std::string>::iterator it;

	it = m_generalConfig.find(key);
	if (it == m_generalConfig.end()) {
		return defval;
	}
	return it->second.c_str();
}

const char* CFileConfig::getString(const char * section, const char* key, const char* defval) {
	std::map<std::string, std::string> *map2 = NULL;
	std::map<std::string, inner_map_t>::iterator it1;
	std::map<std::string, std::string>::iterator it2;

	it1 = m_sectionConfig.find(section);
	if (it1 == m_sectionConfig.end()) {
		return defval;
	}

	map2 = &it1->second;
	it2 = map2->find(key);
	if (it2 == map2->end()) {
		return defval;
	}
	return it2->second.c_str();
}

char* CFileConfig::trim(char* origin, const char* delim) {
	return leftTrim(rightTrim(origin, delim), delim);
}

char* CFileConfig::rightTrim(char* origin, const char* delim) {
	int n = 0;
	int i = 0;

	n = strlen(origin);
	for (i = n; i > 0; i--) {
		if (!strchr(delim, origin[i-1])) {
			origin[i] = 0;
			break;
		}
	}
	return origin;
}

char* CFileConfig::leftTrim(char *origin, const char *delim) {
	char *pOrg = origin;
	char *pOriginCopy = NULL;
	int nFlag = 0;

	if (origin == NULL) {
		return NULL;
	}
	pOriginCopy = origin;

	do {
		if (nFlag == 0) {
			if (strchr(delim, *origin)) {
				origin++;
			} else {
				nFlag = 1;
			}
		} else {
			*pOriginCopy = *origin;
			origin++;
			pOriginCopy++;
		}

	} while (*origin != 0x00);
	*pOriginCopy = *origin;

	return pOrg;
}

} /* namespace kinow */



#ifdef TEST
using namespace kinow;

int main(int argc, char** argv)
{
	const char* file = argv[1];

	IConfig *c = new CFileConfig(file);
	c->load();
	c->show();
	delete c;
}
#endif
