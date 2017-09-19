/*
 * IOUtils.h
 *
 *  Created on: 2016. 9. 23.
 *      Author: root
 */

#ifndef FOOLIB_SRC_IO_IOUTILS_H_
#define FOOLIB_SRC_IO_IOUTILS_H_

#include <stdint.h>

namespace kinow {

class IOUtils {
public:

	/**
	 * extract string content from file
	 * buf size with NULL character.
	 */
	static int extractContent(const char* filename, char *buf, size_t bufsize);
	static int recordContent(const char* filename, char *buf);

private:
	IOUtils();
	virtual ~IOUtils();
};

} /* namespace kinow */

#endif /* FOOLIB_SRC_IO_IOUTILS_H_ */
