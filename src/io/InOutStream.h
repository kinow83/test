/*
 * InOutStream.h
 *
 *  Created on: 2016. 8. 12.
 *      Author: root
 */

#ifndef INOUTSTREAM_H_
#define INOUTSTREAM_H_

#include <stdint.h>
#include <sys/time.h>

namespace kinow {

////////////////////////////////////////////////////
//
////////////////////////////////////////////////////
class InputStream {
public:
	virtual bool read(uint8_t* buf, size_t buflen, size_t& recvlen) = 0;
	virtual bool read(uint8_t* buf, size_t buflen, size_t& recvlen, struct timeval& timeout) = 0;
	virtual bool close() = 0;
};

////////////////////////////////////////////////////
//
////////////////////////////////////////////////////
class OutputStream {
public:
	virtual bool write(uint8_t* buf, size_t sendlen) = 0;
	virtual bool write(uint8_t* buf, size_t sendlen, struct timeval& timeout) = 0;
	virtual bool close() = 0;
};

} /* namespace kinow */

#endif /* INOUTSTREAM_H_ */
