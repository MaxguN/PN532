

#ifndef __SNEP_H__
#define __SNEP_H__

#include "llcp.h"

#define SNEP_DEFAULT_VERSION	0x10	// Major: 1, Minor: 0

#define SNEP_REQUEST_PUT		0x02
#define SNEP_REQUEST_GET		0x01

#define SNEP_RESPONSE_SUCCESS	0x81
#define SNEP_RESPONSE_REJECT	0xFF

class SNEP {
public:
	SNEP(PN532Interface &interface) : llcp(interface) {
		headerBuf = llcp.getHeaderBuffer(&headerBufLen);
	};

	/**
    * @brief    write a SNEP packet, the packet should be less than (255 - 2 - 3) bytes
    * @param    buf     the buffer to contain the packet
    * @param    len     lenght of the buffer
    * @param    timeout max time to wait, 0 means no timeout
    * @return   >0      success
    *			=0      timeout
    *           <0      failed
    */
    int8_t write(const uint8_t *buf, uint8_t len, uint16_t timeout = 0);

    /**
    * @brief    read a SNEP packet, the packet will be less than (255 - 2 - 3) bytes
    * @param    buf     the buffer to contain the packet
    * @param    len     lenght of the buffer
    * @param    timeout max time to wait, 0 means no timeout
    * @return   >=0     length of the packet 
    *           <0      failed
    */
    int16_t read(uint8_t *buf, uint8_t len, uint16_t timeout = 0);

    /**
    * @brief    poll for another peer to communicate with and initiate connection if so
    * @param    timeout max time to wait, 0 means no timeout
    * @return   1       client
    *           2       server
    *           =0      timeout
    *           <0      failed
    */
    int8_t poll(uint16_t timeout = 0);

    int16_t get(uint8_t *buf, uint8_t len, uint8_t maxlen, uint16_t timeout = 0);
    int8_t put(const uint8_t *buf, uint8_t len, uint16_t timeout = 0);
    int16_t serve(uint8_t *buf, uint8_t len, uint16_t timeout = 0);

    /**
    * @brief    terminate connection between peers
    * @return   >0      success
    *           =0      timeout
    *           <0      failed
    */
    int8_t disconnect(uint16_t timeout);

private:
	LLCP llcp;
	uint8_t *headerBuf;
	uint8_t headerBufLen;
};

#endif // __SNEP_H__
