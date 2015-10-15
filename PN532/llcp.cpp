
#include "llcp.h"
#include "PN532_debug.h"

// LLCP PDU Type Values
#define PDU_SYMM    0x00
#define PDU_PAX     0x01
#define PDU_CONNECT 0x04
#define PDU_DISC    0x05
#define PDU_CC      0x06
#define PDU_DM      0x07
#define PDU_I       0x0c
#define PDU_RR      0x0d

#define DEBUG_LLCP

uint8_t LLCP::SYMM_PDU[2] = {0, 0};

inline uint8_t getPType(const uint8_t *buf)
{
    return ((buf[0] & 0x3) << 2) + (buf[1] >> 6);
}

inline uint8_t getSSAP(const uint8_t *buf)
{
    return  buf[1] & 0x3f;
}

inline uint8_t getDSAP(const uint8_t *buf)
{
    return buf[0] >> 2;
}

void print_hex(int16_t length, const uint8_t *data) {
    Serial.print("data read (");
    Serial.print(length);
    Serial.print(")");
    for (int i = 0; i < length; i += 1) {
        Serial.print(" ");Serial.print(data[i], HEX);
    }
    Serial.println("");
}

int8_t LLCP::activate(uint16_t timeout, bool initiator)
{
    isInitiator = initiator;

    if (isInitiator) {
        return link.activateAsInitiator(timeout);
    } else {
        return link.activateAsTarget(timeout);
    }
}

int8_t LLCP::waitForConnection(uint16_t timeout)
{
    uint8_t type;
    int16_t res;

    mode = 1;
    ns = 0;
    nr = 0;

    // Get CONNECT PDU
    DMSG("wait for a CONNECT PDU\n");
    do {
        if (res = link.read(headerBuf, headerBufLen) < 2) {
            return -1;
        }

        type = getPType(headerBuf);
        if (PDU_CONNECT == type) {
            break;
        } else if (PDU_SYMM == type) {
            if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
                return -2;
            }
        } else {
            return -3;
        }

    } while (1);

    // Put CC PDU
    DMSG("put a CC(Connection Complete) PDU to response the CONNECT PDU\n");
    ssap = getDSAP(headerBuf);
    dsap = getSSAP(headerBuf);
    headerBuf[0] = (dsap << 2) + ((PDU_CC >> 2) & 0x3);
    headerBuf[1] = ((PDU_CC & 0x3) << 6) + ssap;
    if (!link.write(headerBuf, 2)) {
        return -2;
    }

    return 1;
}

int8_t LLCP::waitForDisconnection(uint16_t timeout)
{
    uint8_t type;

    // Get DISC PDU
    DMSG("wait for a DISC PDU\n");
    do {
        if (2 > link.read(headerBuf, headerBufLen)) {
            return -1;
        }

        type = getPType(headerBuf);
        if (PDU_DISC == type) {
            break;
        } else if (PDU_SYMM == type) {
            if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
                return -2;
            }
        } else {
            return -3;
        }

    } while (1);

    // Put DM PDU
    DMSG("put a DM(Disconnect Mode) PDU to response the DISC PDU\n");
    // ssap = getDSAP(headerBuf);
    // dsap = getSSAP(headerBuf);
    headerBuf[0] = (dsap << 2) + (PDU_DM >> 2);
    headerBuf[1] = ((PDU_DM & 0x3) << 6) + ssap;
    if (!link.write(headerBuf, 2)) {
        return -2;
    }

    return 1;
}

int8_t LLCP::connect(uint16_t timeout)
{
    uint8_t type;
    int16_t res;

    mode = 0;
    dsap = LLCP_DEFAULT_DSAP;
    ssap = LLCP_DEFAULT_SSAP;
    ns = 0;
    nr = 0;

    //  if (isInitiator) {
    //     if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
    //         Serial.println("Fail write symm ");
    //         return -2;
    //     }
    //  }

    // // try to get a SYMM PDU
    // Serial.println("Before read connect");
    // if (res = link.read(headerBuf, headerBufLen) < 2) {
    //     Serial.println("Fail read symm");
    //     print_hex(res, headerBuf);
    //     return -1;
    // }
    // Serial.println("After read connect");
    // type = getPType(headerBuf);
    // if (PDU_SYMM != type) {
    //     return -1;
    // }

    // put a CONNECT PDU
    headerBuf[0] = (LLCP_DEFAULT_DSAP << 2) + (PDU_CONNECT >> 2);
    headerBuf[1] = ((PDU_CONNECT & 0x03) << 6) + LLCP_DEFAULT_SSAP;
    uint8_t body[] = "  urn:nfc:sn:snep";
    body[0] = 0x06;
    body[1] = sizeof(body) - 2 - 1;
    if (!link.write(headerBuf, 2, body, sizeof(body) - 1)) {
        Serial.println("Fail write CONNECT PDU");
        return -2;
    }

    // wait for a CC PDU
    DMSG("wait for a CC PDU\n");
    do {
        if (2 > link.read(headerBuf, headerBufLen)) {
            return -1;
        }

        type = getPType(headerBuf);
        if (PDU_CC == type) {
            break;
        } else if (PDU_SYMM == type) {
            if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
                return -2;
            }
        } else {
            return -3;
        }

    } while (1);

    return 1;
}

int8_t LLCP::disconnect(uint16_t timeout)
{
    uint8_t type;

    // try to get a SYMM PDU
    if (2 > link.read(headerBuf, headerBufLen)) {
        return -1;
    }
    type = getPType(headerBuf);
    if (PDU_SYMM != type) {
        return -1;
    }

    // put a DISC PDU
    headerBuf[0] = (LLCP_DEFAULT_DSAP << 2) + (PDU_DISC >> 2);
    headerBuf[1] = ((PDU_DISC & 0x03) << 6) + LLCP_DEFAULT_SSAP;
    if (!link.write(headerBuf, 2)) {
        return -2;
    }

    // wait for a DM PDU
    DMSG("wait for a DM PDU\n");
    do {
        if (2 > link.read(headerBuf, headerBufLen)) {
            return -1;
        }

        type = getPType(headerBuf);
        if (PDU_CC == type) {
            break;
        } else if (PDU_DM == type) {
            if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
                return -2;
            }
        } else {
            return -3;
        }

    } while (1);

    return 1;
}

bool LLCP::write(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    uint8_t type;
    uint8_t buf[3];

    if (mode) {
        // Get a SYMM PDU
        if (2 != link.read(buf, sizeof(buf))) {
            return false;
        }
    }

    if (headerBufLen < (hlen + 3)) {
        return false;
    }

    for (int8_t i = hlen - 1; i >= 0; i--) {
        headerBuf[i + 3] = header[i];
    }


    headerBuf[0] = (dsap << 2) + (PDU_I >> 2);
    headerBuf[1] = ((PDU_I & 0x3) << 6) + ssap;
    headerBuf[2] = (ns << 4) + nr;

#ifdef DEBUG_LLCP
    Serial.print((char)0x57);
    for (i = 0; i < 3 + hlen; i += 1) {
        Serial.print((char)0x20);Serial.print(headerBuf[i], HEX);
    }
    for (i = 0; i < blen; i += 1) {
        Serial.print((char)0x20);Serial.print(body[i], HEX);
    }
    Serial.println();
#endif

    if (!link.write(headerBuf, 3 + hlen, body, blen)) {
        return false;
    }

    ns++;

    // Get a RR PDU
    int16_t status;
    do {
        status = link.read(headerBuf, headerBufLen);
        if (2 > status) {
            return false;
        }

        type = getPType(headerBuf);
        if (PDU_RR == type) {
            break;
        } else if (PDU_SYMM == type) {
#ifdef DEBUG_LLCP
            Serial.print((char)0x57);Serial.print((char)0x20);Serial.print((char)0x53);Serial.print((char)0x59);Serial.print((char)0x4D);Serial.print((char)0x4D);
#endif
            if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
                return false;
            }
        } else {
            return false;
        }
    } while (1);

    // if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
    //     Serial.println("llcp write failed");
    //     return false;
    // }

    return true;
}

int16_t LLCP::read(uint8_t *buf, uint8_t length)
{
    uint8_t type;
    uint16_t status;

    // Get INFO PDU
    do {
        status = link.read(buf, length);
        if (2 > status) {
            return -1;
        }

        type = getPType(buf);
        if (PDU_I == type) {
            break;
        } else if (PDU_SYMM == type) {
#ifdef DEBUG_LLCP
            Serial.print((char)0x57);Serial.print((char)0x20);Serial.print((char)0x53);Serial.print((char)0x59);Serial.print((char)0x4D);Serial.print((char)0x4D);
#endif
            if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
                return -2;
            }
        } else {
            return -3;
        }

    } while (1);

    uint8_t len = status - 3;
    ssap = getDSAP(buf);
    dsap = getSSAP(buf);

    headerBuf[0] = (dsap << 2) + (PDU_RR >> 2);
    headerBuf[1] = ((PDU_RR & 0x3) << 6) + ssap;
    headerBuf[2] = (buf[2] >> 4) + 1;
    if (!link.write(headerBuf, 3)) {
        return -2;
    }

    for (uint8_t i = 0; i < len; i++) {
        buf[i] = buf[i + 3];
    }

    nr++;

    return len;
}
