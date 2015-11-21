#include <cstring.h>

#include "mac_link.h"
#include "PN532_debug.h"

int8_t MACLink::activateAsTarget(uint16_t timeout)
{
	pn532.begin();
	pn532.SAMConfig();
	isInitiator = false;
	return pn532.tgInitAsTarget(timeout);
}

int8_t MACLink::activateAsInitiator(uint16_t timeout)
{
	pn532.begin();
	pn532.init();
	pn532.SAMConfig();
	pn532.initiatorInit();
	isInitiator = true;
	return pn532.inJumpForDEP(timeout);
}

bool MACLink::write(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
	DMS("MAC write\n");
	bool result;

	if (isInitiator) {
		uint8_t buffer[64];
		memcpy(buffer, header, hlen);
		if (body) {
			memcpy(buffer + hlen, body, blen);
		}

		linkBufferLength = MAC_BUFFER_SIZE;
		result = pn532.inDataExchange(buffer, hlen + blen, linkBuffer, &linkBufferLength);
		DMSG("inDataExchange done\n");
	} else {
		result = pn532.tgSetData(header, hlen, body, blen);
		DMSG("tgSetData done\n");
	}

	return result;
}

int16_t MACLink::read(uint8_t *buf, uint8_t len)
{
	DMS("MAC read\n");
	int16_t res;

	if (isInitiator) {
		memcpy(buf, linkBuffer, linkBufferLength);
		res = linkBufferLength;

		linkBufferLength = 0;
	} else {
		res = pn532.tgGetData(buf, len);
	}

	return res;
}
