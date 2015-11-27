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
	// pn532.init();
	pn532.SAMConfig();
	// pn532.initiatorInit();
	isInitiator = true;
	return pn532.inJumpForDEP(timeout);
}

bool MACLink::write(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
	DMS("MAC write\n");
	bool result;

	if (isInitiator) {
		result = pn532.inDataWrite(header, hlen, body, blen);
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
	int16_t result;

	if (isInitiator) {
		if (!pn532.inDataRead(buf, &len)) {
			return -1;
		}

		result = len;
	} else {
		result = pn532.tgGetData(buf, len);
	}

	return result;
}
