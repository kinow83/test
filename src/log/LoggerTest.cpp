/*
 * LoggerTest.cpp
 *
 *  Created on: 2016. 8. 5.
 *      Author: root
 */


#include <stdio.h>
#include "CConsoleLog.h"
#include "CFileLog.h"

using namespace kinow;

int main() {
	const char* desc = "kaka kinow 123456 GoGo ~~ !!!";
	byte hex[64];

	for (int i=0; i<sizeof(hex); i++) {
		hex[i] = i;
	}

	ILogger *l = new CFileLog(L_ALL, ".", "kaka", new CConsoleLog(L_ALL));

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

