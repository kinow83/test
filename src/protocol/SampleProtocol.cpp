/*
 * SampleProtocol.cpp
 *
 *  Created on: 2016. 8. 11.
 *      Author: root
 */

#include <stdio.h>
#include "SampleProtocol.h"

namespace kinow {

SampleProtocol::SampleProtocol()
: CProtocol(hello_last) {

	registerCallback(client_hello_t,
			bind(&SampleProtocol::clientHello, this, std::placeholders::_1));
	registerCallback(server_hello_t,
			bind(&SampleProtocol::serverHello, this, std::placeholders::_1));
	registerCallback(authenticate_t,
			bind(&SampleProtocol::authenticate, this, std::placeholders::_1));
	registerCallback(hello_done_t,
			bind(&SampleProtocol::helloDone, this, std::placeholders::_1));

	registerPrevCallback(bind(&SampleProtocol::protoValidate, this, std::placeholders::_1));
	registerPostCallback(bind(&SampleProtocol::postDoing, this, std::placeholders::_1));
}

SampleProtocol::~SampleProtocol() {

}

bool SampleProtocol::protoValidate(const ProtoTLV& tlv) {
	printf("[prev] call protoValidate\n");
	return true;
}

bool SampleProtocol::postDoing(const ProtoTLV& tlv) {
	printf("[post] call postDoing\n");
	return true;
}

bool SampleProtocol::clientHello(const ProtoTLV& tlv) {
	printf("call clientHello\n");
	return true;
}

bool SampleProtocol::serverHello(const ProtoTLV& tlv) {
	printf("call serverHello\n");
	return true;
}

bool SampleProtocol::authenticate(const ProtoTLV& tlv) {
	printf("call authenticate\n");
	return true;
}

bool SampleProtocol::helloDone(const ProtoTLV& tlv) {
	printf("call helloDone\n");
	return true;
}

bool SampleProtocol::analyze(uint8_t* pdata, size_t datalen) {
	ProtoTLV tlv;

	callback(client_hello_t, tlv);
	callback(server_hello_t, tlv);
	callback(authenticate_t, tlv);
	callback(hello_done_t, tlv);

	return true;
}


} /* namespace kinow */


using namespace kinow;

int main(int argc, char** argv) {
	SampleProtocol proto;

	proto.analyze(NULL, 0);
}
