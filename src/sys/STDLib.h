/*
 * STDLib.h
 *
 *  Created on: 2016. 8. 3.
 *      Author: root
 */

#ifndef STDLIB_H_
#define STDLIB_H_

namespace kinow {

class STDLib {
public:
	static void closeAllFileDescriptions();
	static int safeSystem(const char* cmd);

private:
	STDLib();
	virtual ~STDLib();
};

} /* namespace kinow */

#endif /* STDLIB_H_ */
