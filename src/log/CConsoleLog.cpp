/*
 * CConsoleLog.cpp
 *
 *  Created on: 2016. 8. 4.
 *      Author: root
 */

#include <stdio.h>
#include <time.h>
#include "CConsoleLog.h"

namespace kinow {

/*
<List of colors for prompt and Bash>

txtblk='\e[0;30m' # Black - Regular
txtred='\e[0;31m' # Red
txtgrn='\e[0;32m' # Green
txtylw='\e[0;33m' # Yellow
txtblu='\e[0;34m' # Blue
txtpur='\e[0;35m' # Purple
txtcyn='\e[0;36m' # Cyan
txtwht='\e[0;37m' # White
bldblk='\e[1;30m' # Black - Bold
bldred='\e[1;31m' # Red
bldgrn='\e[1;32m' # Green
bldylw='\e[1;33m' # Yellow
bldblu='\e[1;34m' # Blue
bldpur='\e[1;35m' # Purple
bldcyn='\e[1;36m' # Cyan
bldwht='\e[1;37m' # White
unkblk='\e[4;30m' # Black - Underline
undred='\e[4;31m' # Red
undgrn='\e[4;32m' # Green
undylw='\e[4;33m' # Yellow
undblu='\e[4;34m' # Blue
undpur='\e[4;35m' # Purple
undcyn='\e[4;36m' # Cyan
undwht='\e[4;37m' # White
bakblk='\e[40m'   # Black - Background
bakred='\e[41m'   # Red
bakgrn='\e[42m'   # Green
bakylw='\e[43m'   # Yellow
bakblu='\e[44m'   # Blue
bakpur='\e[45m'   # Purple
bakcyn='\e[46m'   # Cyan
bakwht='\e[47m'   # White
txtrst='\e[0m'    # Text Reset
 */
#ifndef LOG_NO_COLOR
	#define VTC_BLUE   "\x1b[34m" //!< Colour following text blue.
	#define VTC_RED    "\x1b[31m" //!< Colour following text red.
	#define VTC_GREEN  "\x1b[32m" //!< Colour following text creen.
	#define VTC_YELLOW "\x1b[33m" //!< Colour following text yellow.
	#define VTC_BOLD   "\x1b[1m" //!< Embolden following text.
	#define VTC_RESET  "\x1b[0m" //!< Reset terminal text to default style/colour.
#endif

void CConsoleLog::fireCri(const char* msg) {
	fireLog(L_CRI, msg, NULL, false);
	fireNextCri(msg);
}
void CConsoleLog::fireCRI(const char* msg) {
	fireLog(L_CRI, msg, NULL, true);
	fireNextCRI(msg);
}
void CConsoleLog::fireErr(const char* msg) {
	fireLog(L_ERR, msg, NULL, false);
	fireNextErr(msg);
}
void CConsoleLog::fireERR(const char* msg) {
	fireLog(L_ERR, msg, NULL, true);
	fireNextERR(msg);
}
void CConsoleLog::fireInf(const char* msg) {
	fireLog(L_INF, msg, NULL, false);
	fireNextInf(msg);
}
void CConsoleLog::fireINF(const char* msg) {
	fireLog(L_INF, msg, NULL, true);
	fireNextINF(msg);
}
void CConsoleLog::fireDbg(const char* msg) {
	fireLog(L_DBG, msg, NULL, false);
	fireNextDbg(msg);
}
void CConsoleLog::fireDBG(const char* msg) {
	fireLog(L_DBG, msg, NULL, true);
	fireNextDBG(msg);
}
void CConsoleLog::fireCriHex(byte* hex, int len, const char* msg) {
	fireLogHex(L_CRI, hex, len, msg, NULL, false);
	fireNextCriHex(hex, len, msg);
}
void CConsoleLog::fireErrHex(byte* hex, int len, const char* msg) {
	fireLogHex(L_ERR, hex, len, msg, NULL, false);
	fireNextErrHex(hex, len, msg);
}
void CConsoleLog::fireInfHex(byte* hex, int len, const char* msg) {
	fireLogHex(L_INF, hex, len, msg, NULL, false);
	fireNextInfHex(hex, len, msg);
}
void CConsoleLog::fireDbgHex(byte* hex, int len, const char* msg) {
	fireLogHex(L_DBG, hex, len, msg, NULL, false);
	fireNextDbgHex(hex, len, msg);
}

void CConsoleLog::fireLog(L_LV lvl, const char* msg, const char* color, bool bold) {
	struct tm stNow;
	char szTime[20] = {0,};
	time_t now = time(NULL);
	const char* pLvl;
	const char* pColor;

	localtime_r(&now, &stNow);
	strftime(szTime, sizeof(szTime), "%H:%M:%S", &stNow);

	switch (lvl) {
	case L_CRI:
		pLvl = "CRI";
		pColor = VTC_YELLOW;
		break;
	case L_ERR:
		pLvl = "ERR";
		pColor = VTC_RED;
		break;
	case L_INF:
		pLvl = "INF";
		pColor = VTC_GREEN;
		break;
	case L_DBG:
		pLvl = "DBG";
		pColor = VTC_RESET;
		break;
	}
	fprintf(stdout, "%s%s%s :CRITI: %s\n", color ? color : pColor, (bold ? VTC_BOLD : ""), szTime, msg);
}

void CConsoleLog::fireLogHex(L_LV lvl, byte* hex, int len, const char* msg, const char* color, bool bold) {
	int i;

	fireLog(lvl, msg, color, bold);

	fprintf(stdout, "len: %d\n", len);
	for (i=1; i<=len; i++) {
		fprintf(stdout, "%02x ", hex[i-1]);
		if (i%8 == 0) fprintf(stdout, " ");
		if (i%16 == 0) fprintf(stdout, "\n");
	}
	fprintf(stdout, "\n");
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

	ILogger *l = new CConsoleLog(L_ALL);

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
