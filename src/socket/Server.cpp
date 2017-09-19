/*
 * Server.cpp
 *
 *  Created on: 2016. 8. 11.
 *      Author: root
 */

typedef unsigned char byte;

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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


template <typename T>
class CProtocol {
public:
	typedef bool (T::*protoCallback)(tlv_t&);

public:
	virtual bool analyze(uint8_t* pdata, size_t datalen) = 0;
protected:
	protoCallback *m_pProtoCallback;
	void setCallback(int n, protoCallback cb) {
		m_pProtoCallback[n] = cb;
	}
	protoCallback& getCallback(int n) {
		return m_pProtoCallback[n];
	}
	CProtocol(int protoCount) : m_nProtoCount(protoCount) {
		m_pProtoCallback = (protoCallback*)malloc(sizeof(protoCallback) * m_nProtoCount);
	}
	~CProtocol() {
		if (m_pProtoCallback) {
			free(m_pProtoCallback);
		}
	}
private:
	int m_nProtoCount;
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


class CHelloProtocol: public CProtocol<CHelloProtocol> {
public:
	bool clientHelloCB(tlv_t& tlv) {
		return true;
	}
	bool serverHelloCB(tlv_t& tlv) {
		return false;
	}
	CHelloProtocol() : CProtocol(type_hello_last) {
		setCallback(type_client_hello, clientHelloCB);
	}
	virtual bool analyze(uint8_t* pdata, size_t datalen) {
		return true;
	}
};

#error 111111111111

int main() {
	CHelloProtocol proto;
}

