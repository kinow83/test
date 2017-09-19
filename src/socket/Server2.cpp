/*
 * Server.cpp
 *
 *  Created on: 2016. 8. 11.
 *      Author: root
 */

//#define __cplusplus 201103L

#include <functional>
#include <iostream>

using namespace std;

/////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////
class ITransceiver {
public:
	virtual bool handshake() = 0;
	virtual bool send(uint8_t* buf, size_t buflen, int timeout) = 0;
	virtual bool recv(uint8_t* buf, size_t buflen, size_t& recvlen, int timeout) = 0;
private:
	virtual ~ITransceiver();
};

/////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////
typedef struct st_tlv {
	uint16_t type;
	uint32_t length;
	uint8_t* pdata;
} tlv_t;



typedef function<bool(tlv_t&)> pcb_t;

class CProtocol {
public:
	virtual bool analyze(uint8_t* pdata, size_t datalen) = 0;

protected:
	void setCB(int n, pcb_t callback) {
		pcblist[n] = move(callback);
	}
	const pcb_t& getCB(int n) const {
		return pcblist[n];
	}
	CProtocol(int protoCount) : m_nProtoCount(protoCount) {
		pcblist = new pcb_t[m_nProtoCount];
	}
	~CProtocol() {
		delete [] pcblist;
	}
	void debugTLV(tlv_t& tlv) {
		cout << " type: " << tlv.type;
		cout << " len : " << tlv.length;
		cout << endl;
	}
private:
	int m_nProtoCount;
	pcb_t *pcblist;
};


/////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////
typedef enum {
	type_client_hello = 0,
	type_server_hello = 1,
	type_authenticate = 2,
	type_hello_done   = 3,
	type_hello_last   = 4
} PROTO_HELLO_BODY;


class CHelloProtocol: public CProtocol {
public:
	bool clientHelloCB(tlv_t& tlv) {
		cout << "call clientHelloCB" << endl;
		debugTLV(tlv);
		return true;
	}
	bool serverHelloCB(tlv_t& tlv) {
		cout << "call serverHelloCB" << endl;
		debugTLV(tlv);
		return false;
	}
	CHelloProtocol() : CProtocol(type_hello_last) {
		setCB(type_client_hello, bind(&CHelloProtocol::clientHelloCB, this, placeholders::_1));
		setCB(type_server_hello, bind(&CHelloProtocol::serverHelloCB, this, placeholders::_1));
	}

	virtual bool analyze(uint8_t* pdata, size_t datalen) {
		tlv_t tlv;
		pcb_t cb;


		tlv.type = type_client_hello;
		tlv.length = 4;
		tlv.pdata = pdata;
		cb = getCB(type_client_hello);
		cb(tlv);


		tlv.type = type_server_hello;
		tlv.length = 4;
		tlv.pdata = pdata;
		cb = getCB(type_server_hello);
		cb(tlv);

		return true;
	}
};

int main() {
	CHelloProtocol proto;

	proto.analyze(NULL, 10);
}

