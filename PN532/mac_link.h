

#ifndef __MAC_LINK_H__
#define __MAC_LINK_H__

#include "PN532.h"

#define MAC_BUFFER_SIZE 64

class MACLink {
public:
    MACLink(PN532Interface &interface) : pn532(interface), isInitiator(false) {

    };
    
    /**
    * @brief    Activate PN532 as a target
    * @param    timeout max time to wait, 0 means no timeout
    * @return   > 0     success
    *           = 0     timeout
    *           < 0     failed
    */
    int8_t activateAsTarget(uint16_t timeout = 0);
    
    /**
    * @brief    Activate PN532 as an initiator
    * @param    timeout max time to wait, 0 means no timeout
    * @return   > 0     success
    *           = 0     timeout
    *           < 0     failed
    */
    int8_t activateAsInitiator(uint16_t timeout = 0);

    /**
    * @brief    write a PDU packet, the packet should be less than (255 - 2) bytes
    * @param    header  packet header
    * @param    hlen    length of header
    * @param 	body	packet body
    * @param 	blen	length of body
    * @return   true    success
    *           false   failed
    */
    int16_t write(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);

    /**
    * @brief    read a PDU packet, the packet will be less than (255 - 2) bytes
    * @param    buf     the buffer to contain the PDU packet
    * @param    len     lenght of the buffer
    * @return   >=0     length of the PDU packet 
    *           <0      failed
    */
    int16_t read(uint8_t *buf, uint8_t len);

    uint8_t *getHeaderBuffer(uint8_t *len) {
        return pn532.getBuffer(len);
    };
    
private:
    PN532 pn532;
    uint8_t linkBuffer[MAC_BUFFER_SIZE];
    uint8_t linkBufferLength;
    bool isInitiator;
};

#endif // __MAC_LINK_H__
