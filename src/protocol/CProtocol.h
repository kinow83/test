/*
 * CProtocol.h
 *
 *  Created on: 2016. 8. 11.
 *      Author: root
 */

#ifndef CPROTOCOL_H_
#define CPROTOCOL_H_

////////////////////////////////////////////
// MUST compile C++11
////////////////////////////////////////////

#include <functional>

namespace kinow {

/////////////////////////////
// class: ProtoTLV
/////////////////////////////
class ProtoTLV {
public:
	uint16_t t;
	uint32_t l;
	uint8_t* v;
	bool length0() { return (l==0); }
	bool validate() {
		if ((l && !v) || (!l && v)) {
			return false;
		}
		return true;
	}
public:
	void debug();
};

typedef std::function<bool(const ProtoTLV&)> callback_t;

/////////////////////////////
// class: CProtocol
/////////////////////////////
class CProtocol {
public:
	virtual bool analyze(uint8_t* pdata, size_t datalen) = 0;

protected:
	void registerPrevCallback(callback_t callback);
	void registerPostCallback(callback_t callback);
	void registerCallback(int n, callback_t callback);

	bool callback(int n, const ProtoTLV& tlv);
	bool prevCallback(const ProtoTLV& tlv);
	bool postCallback(const ProtoTLV& tlv);

	const callback_t& getCallback(int n) const;

protected:
	CProtocol(int callbackSize);
	virtual ~CProtocol();

private:
	int m_nCallbackSize;
	callback_t* m_pCallbackList;
	callback_t m_prevCallback;
	callback_t m_postCallback;
};

} /* namespace kinow */

#endif /* CPROTOCOL_H_ */
