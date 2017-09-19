/*
 * CProtocol.cpp
 *
 *  Created on: 2016. 8. 11.
 *      Author: root
 */

#include <stdio.h>
#include "CProtocol.h"

namespace kinow {

void ProtoTLV::debug() {
	int i = 0;

	printf("type: %d"
			"len : %d\n", t, l);
	for (i=1; i<=l; i++) {
		if (i % 16 == 0) {
			printf("%02x\n", v[i]);
		} else if (i % 8 == 0) {
			printf("%02x    ", v[i]);
		} else {
			printf("%02x ", v[i]);
		}
	}
	printf("\n");

}

CProtocol::CProtocol(int callbackSize) :
		m_nCallbackSize(callbackSize),
		m_prevCallback(NULL),
		m_postCallback(NULL)
{
	int i;
	m_pCallbackList = new callback_t[m_nCallbackSize];
	for (i=0; i<m_nCallbackSize; i++) {
		m_pCallbackList[i] = NULL;
	}
}

CProtocol::~CProtocol() {
	delete [] m_pCallbackList;
}

void CProtocol::registerPrevCallback(callback_t callback) {
	m_prevCallback = std::move(callback);
}

void CProtocol::registerPostCallback(callback_t callback) {
	m_postCallback = std::move(callback);
}

void CProtocol::registerCallback(int n, callback_t callback) {
	if (n >= m_nCallbackSize) {
		// throw exception
		fprintf(stderr, "Failed register callback - index(%d) over than size(%d)\n",
				n, m_nCallbackSize);
		return;
	}
	if (m_pCallbackList[n]) {
		// throw exception (already bind)
		fprintf(stderr, "Aleady bin #%d of callback\n", n);
		return;
	}
	m_pCallbackList[n] = std::move(callback);
	printf("registered #%d of callback\n", n);
}

const callback_t& CProtocol::getCallback(int n) const {
	if (n >= m_nCallbackSize) {
		// throw exception
		return NULL;
	}
	return m_pCallbackList[n];
}

bool CProtocol::callback(int n, const ProtoTLV& tlv) {
	callback_t cb = NULL;

	cb = getCallback(n);
	if (cb == NULL) {
		return false;
	}

	return prevCallback(tlv) &&
			cb(tlv) &&
			postCallback(tlv);
}

bool CProtocol::prevCallback(const ProtoTLV& tlv) {
	if (!m_prevCallback) {
		return true;
	}
	return m_prevCallback(tlv);
}

bool CProtocol::postCallback(const ProtoTLV& tlv) {
	if (!m_postCallback) {
		return true;
	}
	return m_postCallback(tlv);
}

} /* namespace kinow */
