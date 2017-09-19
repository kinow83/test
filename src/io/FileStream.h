/*
 * FileStream.h
 *
 *  Created on: 2016. 8. 12.
 *      Author: root
 */

#ifndef FILESTREAM_H_
#define FILESTREAM_H_

namespace kinow {

class FileStream {
public:
	virtual bool read(uint8_t* buf, size_t buflen, size_t& recvlen);
	virtual bool read(uint8_t* buf, size_t buflen, size_t& recvlen, struct timeval& timeout);
	virtual bool close();
public:
	virtual bool readLine(uint8_t* buf, size_t buflen, size_t& recvlen);
public:
	FileStream();
	virtual ~FileStream();
};

} /* namespace kinow */

#endif /* FILESTREAM_H_ */
