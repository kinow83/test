/*
 * SampleProtocol.h
 *
 *  Created on: 2016. 8. 11.
 *      Author: root
 */

#ifndef SAMPLEPROTOCOL_H_
#define SAMPLEPROTOCOL_H_

#include "CProtocol.h"

namespace kinow {

typedef enum {
	client_hello_t = 0,
	server_hello_t,
	authenticate_t,
	hello_done_t,
	hello_last,
} H_PROTO_BODY;

class SampleProtocol : public CProtocol {
public:
	virtual bool analyze(uint8_t* pdata, size_t datalen);

public:
	SampleProtocol();
	virtual ~SampleProtocol();

private:
	bool protoValidate(const ProtoTLV& tlv);
	bool postDoing(const ProtoTLV& tlv);
	bool clientHello(const ProtoTLV& tlv);
	bool serverHello(const ProtoTLV& tlv);
	bool authenticate(const ProtoTLV& tlv);
	bool helloDone(const ProtoTLV& tlv);
};

} /* namespace kinow */

#endif /* SAMPLEPROTOCOL_H_ */
