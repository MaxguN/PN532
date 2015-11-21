
#include "snep.h"
#include "PN532_debug.h"

#define DEBUG_SNEP

int8_t SNEP::write(const uint8_t *buf, uint8_t len, uint16_t timeout)
{
    DMS("Starting write\n");
	if (0 >= llcp.activate(timeout)) {
		DMSG("failed to activate PN532 as a target\n");
		return -1;
	}
    DMS("llcp activated\n");

	if (0 >= llcp.connect(timeout)) {
		DMSG("failed to set up a connection\n");
		return -2;
	}
    DMS("llcp connected\n");

	// response a success SNEP message
	headerBuf[0] = SNEP_DEFAULT_VERSION;
	headerBuf[1] = SNEP_REQUEST_PUT;
	headerBuf[2] = 0;
	headerBuf[3] = 0;
	headerBuf[4] = 0;
	headerBuf[5] = len;
	if (0 >= llcp.write(headerBuf, 6, buf, len)) {
		return -3;
	}

	uint8_t rbuf[16];
	if (6 > llcp.read(rbuf, sizeof(rbuf))) {
		return -4;
	}

	// check SNEP version
	if (SNEP_DEFAULT_VERSION != rbuf[0]) {
		DMSG(F("The received SNEP message's major version is different, me: "));
		DMSG(SNEP_DEFAULT_VERSION);
		DMSG(", their: ");
		DMSG(rbuf[0]);
		DMSG("\n");
		// To-do: send Unsupported Version response
		return -4;
	}

	// expect a put request
	if (SNEP_RESPONSE_SUCCESS != rbuf[1]) {
		DMSG("Expect a success response\n");
		return -4;
	}

	llcp.disconnect(timeout);

	return 1;
}

int16_t SNEP::read(uint8_t *buf, uint8_t len, uint16_t timeout)
{
    DMS("Starting read\n");
	if (0 >= llcp.activate(timeout)) {
		DMSG("failed to activate PN532 as a target\n");
		return -1;
	}
    DMS("llcp activated\n");

	if (0 >= llcp.waitForConnection(timeout)) {
		DMSG("failed to set up a connection\n");
		return -2;
	}
    DMS("llcp connected\n");

	uint16_t status = llcp.read(buf, len);
	if (6 > status) {
		return -3;
	}


	// check SNEP version
	
	// in case of platform specific bug, shift SNEP message for 4 bytes.
	// tested on Nexus 5, Android 5.1
	if (SNEP_DEFAULT_VERSION != buf[0] && SNEP_DEFAULT_VERSION == buf[4]) {
		for (uint8_t i = 0; i < len - 4; i++) {
			buf[i] = buf[i + 4];
		}
	}
	
	if (SNEP_DEFAULT_VERSION != buf[0]) {
		DMSG(F("SNEP->read: The received SNEP message's major version is different, me: "));
		DMSG(SNEP_DEFAULT_VERSION);
		DMSG(", their: ");
		DMSG(buf[0]);
		DMSG("\n");
		// To-do: send Unsupported Version response
		return -4;
	}

	// expect a put request
	if (SNEP_REQUEST_PUT != buf[1]) {
		DMSG("Expect a put request\n");
		return -4;
	}

	// check message's length
	uint32_t length = (buf[2] << 24) + (buf[3] << 16) + (buf[4] << 8) + buf[5];
	// length should not be more than 244 (header + body < 255, header = 6 + 3 + 2)
	if (length > (status - 6)) {
		DMSG("The SNEP message is too large: "); 
		DMSG_INT(length);
		DMSG_INT(status - 6);
		DMSG("\n");
		return -4;
	}
	for (uint8_t i = 0; i < length; i++) {
		buf[i] = buf[i + 6];
	}

	// response a success SNEP message
	headerBuf[0] = SNEP_DEFAULT_VERSION;
	headerBuf[1] = SNEP_RESPONSE_SUCCESS;
	headerBuf[2] = 0;
	headerBuf[3] = 0;
	headerBuf[4] = 0;
	headerBuf[5] = 0;
	llcp.write(headerBuf, 6);

	return length;
}

int8_t SNEP::poll(uint16_t timeout) {
	int8_t result;
	bool loop = false;

    if (timeout == 0) {
    	timeout = 500;
    	loop = true;
    }

    do {
    	result = llcp.activate(timeout >> 1, true);
    	if (result < 0) {
			DMSG("failed to activate PN532 as an initiator\n");
			return -1;
		} else if (result > 0) {
			result = 1;
			break;
		}

    	result = llcp.activate(timeout / 2, false);
    	if (result < 0) {
			DMSG("failed to activate PN532 as a target\n");
			return -1;
		} else if (result > 0) {
			result = 2;
			break;
		}
    } while (loop);

    if (result == 0) {
    	return result;
    }
	

    if (result == 1) {
    	if (0 >= llcp.connect(timeout)) {
			DMSG("failed to set up a connection\n");
			return -2;
		}
    } else if (result == 2) {
		if (0 >= llcp.waitForConnection(timeout)) {
			DMSG("failed to set up a connection\n");
			return -2;
		}
    }

    return result;
}

int16_t SNEP::get(uint8_t *buf, uint8_t len, uint8_t maxlen, uint16_t timeout) {
	uint8_t i;

	DMS("get\n");
	headerBuf[0] = SNEP_DEFAULT_VERSION;
	headerBuf[1] = SNEP_REQUEST_GET;
	headerBuf[2] = 0;
	headerBuf[3] = 0;
	headerBuf[4] = 0;
	headerBuf[5] = len + 4;
	headerBuf[6] = 0;
	headerBuf[7] = 0;
	headerBuf[8] = 0;
	headerBuf[9] = maxlen;

#ifdef DEBUG_SNEP
	Serial.print((char)0x57);
	for (i = 0; i < 10; i += 1) {
		DMS_HEX(headerBuf[i]);
	}
	for (i = 0; i < len; i += 1) {
		DMS_HEX(buf[i]);
	}
	DMS("\n");
#endif

	if (0 >= llcp.write(headerBuf, 10, buf, len)) {
		return -3;
	}

	uint8_t rbuf[128];
	if (6 > llcp.read(rbuf, sizeof(rbuf))) {
		return -4;
	}

	// check SNEP version
	if (SNEP_DEFAULT_VERSION != rbuf[0]) {
		DMSG("The received SNEP message's major version is different\n");
		// To-do: send Unsupported Version response
		return -4;
	}

	// expect a put request
	if (SNEP_RESPONSE_SUCCESS != rbuf[1]) {
		DMSG("Expect a success response\n");
		return -4;
	}

	uint8_t length;
	length = rbuf[5];

#ifdef DEBUG_SNEP
	Serial.print((char)0x52);
	for (i = 0; i < length + 6; i += 1) {
		DMS_HEX(rbuf[i]);
	}
	DMS("\n");
#endif

	for (i = 0; i < length; i += 1) {
		buf[i] = rbuf[i + 6];
	}

	return length;
}

int8_t SNEP::put(const uint8_t *buf, uint8_t len, uint16_t timeout) {
	uint8_t i;

	DMS("put\n");

	headerBuf[0] = SNEP_DEFAULT_VERSION;
	headerBuf[1] = SNEP_REQUEST_PUT;
	headerBuf[2] = 0;
	headerBuf[3] = 0;
	headerBuf[4] = 0;
	headerBuf[5] = len;

#ifdef DEBUG_SNEP
	Serial.print((char)0x57);
	for (i = 0; i < 6; i += 1) {
		DMS_HEX(headerBuf[i]);
	}
	for (i = 0; i < len; i += 1) {
		DMS_HEX(buf[i]);
	}
	DMS("\n");
#endif

	if (0 >= llcp.write(headerBuf, 6, buf, len)) {
		DMS(-3);
		return -3;
	}

	uint8_t rbuf[16];
	if (6 > llcp.read(rbuf, sizeof(rbuf))) {
		DMS(-4);
		return -4;
	}

	// check SNEP version
	if (SNEP_DEFAULT_VERSION != rbuf[0]) {
		DMSG("The received SNEP message's major version is different\n");
		// To-do: send Unsupported Version response
		DMS(-5);
		return -5;
	}

	// expect a put request
	if (SNEP_RESPONSE_SUCCESS != rbuf[1]) {
		DMSG("Expect a success response\n");
		DMS(-6);
		return -6;
	}

#ifdef DEBUG_SNEP
	Serial.print((char)0x52);
	for (i = 0; i < 6; i += 1) {
		DMS_HEX(rbuf[i]);
	}
	DMS("\n");
#endif

	return 1;
}

int16_t SNEP::serve(uint8_t *buf, uint8_t len, uint16_t timeout) {
	uint8_t rbuf[128];
	uint8_t i;

	DMS("serve\n");
	if (6 > llcp.read(rbuf, sizeof(rbuf))) {
		return -4;
	}

	// check SNEP version
	if (SNEP_DEFAULT_VERSION != rbuf[0]) {
		DMSG("The received SNEP message's major version is different\n");
		// To-do: send Unsupported Version response
		return -4;
	}

	uint8_t length;
	length = rbuf[5];

#ifdef DEBUG_SNEP
	Serial.print((char)0x52);
	for (i = 0; i < length + 6; i += 1) {
		DMS_HEX(rbuf[i]);
	}
	DMS("\n");
#endif

	if (rbuf[1] == SNEP_REQUEST_GET) {
		length = rbuf[9];

		if (len < length) {
			DMSG("Message too big for client\n");
			return -4;
		}

		headerBuf[0] = SNEP_DEFAULT_VERSION;
		headerBuf[1] = SNEP_RESPONSE_SUCCESS;
		headerBuf[2] = 0;
		headerBuf[3] = 0;
		headerBuf[4] = 0;
		headerBuf[5] = len;

#ifdef DEBUG_SNEP
		Serial.print((char)0x57);
		for (i = 0; i < 6; i += 1) {
			DMS_HEX(headerBuf[i]);
		}
		for (i = 0; i < len; i += 1) {
			DMS_HEX(buf[i]);
		}
		DMS("\n");
#endif

		llcp.write(headerBuf, 6, buf, len);

		return 1;
	} else if (rbuf[1] == SNEP_REQUEST_PUT) {
		for (i = 0; i < length; i += 1) {
			buf[i] = rbuf[i + 6];
		}

		headerBuf[0] = SNEP_DEFAULT_VERSION;
		headerBuf[1] = SNEP_RESPONSE_SUCCESS;
		headerBuf[2] = 0;
		headerBuf[3] = 0;
		headerBuf[4] = 0;
		headerBuf[5] = 0;

#ifdef DEBUG_SNEP
		Serial.print((char)0x57);
		for (i = 0; i < 6; i += 1) {
			DMS_HEX(headerBuf[i]);
		}
		DMS("\n");

		llcp.write(headerBuf, 6);
#endif

		return length;
	} else {
		DMSG("Expect a put or get request\n");
		return -4;
	}

	return -1;
}

int8_t SNEP::disconnect(uint16_t timeout) {
	return llcp.disconnect(timeout);
}
