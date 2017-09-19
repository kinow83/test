/*
 * NetStream.h
 *
 *  Created on: 2016. 8. 12.
 *      Author: root
 */

#ifndef NETSTREAM_H_
#define NETSTREAM_H_

#include <sys/socket.h>
#include "InOutStream.h"

namespace kinow {


////////////////////////////////////////////////////
//
////////////////////////////////////////////////////
class NetInputStream : public InputStream {

public: // InputStream
	virtual bool read(uint8_t* buf, size_t buflen, size_t& recvlen);
	virtual bool read(uint8_t* buf, size_t buflen, size_t& recvlen, struct timeval& timeout);
	virtual bool close();

public:
	NetInputStream();
	virtual ~NetInputStream();
};

////////////////////////////////////////////////////
//
////////////////////////////////////////////////////
class NetOutputStream: public OutputStream {

};

} /* namespace kinow */

#endif /* NETSTREAM_H_ */
