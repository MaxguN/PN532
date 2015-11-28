// Exchange NDEF Messages between Peers
// Requires SPI. Tested with Seeed Studio NFC Shield v2

#include "SPI.h"
#include "PN532_SPI.h"
#include "snep.h"
#include "NdefMessage.h"

#define QA_CHARACTER 0x00
#define QA_RESULT    0x01
#define QA_HEAL      0x02

PN532_SPI pn532spi(SPI, 10);
SNEP nfc(pn532spi);
uint8_t ndefBuf[128];
uint8_t header[] = {0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x3a, 0x00};
uint8_t o1Data[] = {0x00, 0x01, 0x02, 0x03, 0x04};
uint8_t o2Data[] = {0x05, 0x06, 0x07, 0x08, 0x09};

uint8_t setCharNdef(uint8_t *buffer, const uint8_t *character, uint8_t length);
uint8_t setRequestNdef(uint8_t *buffer, uint8_t request);
uint8_t setActionNdef(uint8_t *buffer, uint8_t action);
void print_ndef(const uint8_t *buffer, uint8_t length);

void setup() {
    Serial.begin(115200);
}

void loop() {
    int16_t result;
    uint8_t length;
    
    result = nfc.poll();
    
    if (result == 1) { // client
      Serial.println("Client peer");
    
      length = setCharNdef(ndefBuf, o1Data, sizeof(o1Data));
      nfc.put(ndefBuf, length); // send character data
      
      length = setRequestNdef(ndefBuf, QA_CHARACTER);
      result = nfc.get(ndefBuf, length, sizeof(ndefBuf)); // get peer character data

      // process
      print_ndef(ndefBuf, result);

      length = setActionNdef(ndefBuf, QA_HEAL);
      nfc.put(ndefBuf, length); // send result
 
      length = setRequestNdef(ndefBuf, QA_RESULT);
      result = nfc.get(ndefBuf, length, sizeof(ndefBuf)); // get peer confirmation
 
      print_ndef(ndefBuf, result);
    } else if (result == 2) { // server
      Serial.println("Server peer");

      result = nfc.serve(ndefBuf, sizeof(ndefBuf)); // get peer character data
      print_ndef(ndefBuf, result);
      
      length = setCharNdef(ndefBuf, o2Data, sizeof(o2Data));
      nfc.serve(ndefBuf, length); // send character data

      // process

      result = nfc.serve(ndefBuf, sizeof(ndefBuf)); // get peer result
      print_ndef(ndefBuf, result);

      length = setActionNdef(ndefBuf, QA_HEAL);
      nfc.serve(ndefBuf, length); // send confirmation
    }
    
    delay(3000);
}

uint8_t setCharNdef(uint8_t *buffer, const uint8_t *character, uint8_t length) {
  buffer[0] = 0xD4;
  buffer[1] = sizeof(header);
  buffer[2] = length;
  
  header[buffer[1] - 1] = 0x43;
  
  memcpy(buffer + 3, header, buffer[1]);
  memcpy(buffer + 3 + buffer[1], character, buffer[2]);
  
  return 3 + buffer[1] + buffer[2];
}

uint8_t setRequestNdef(uint8_t *buffer, uint8_t request) {
  buffer[0] = 0xD4;
  buffer[1] = sizeof(header);
  buffer[2] = 0x01;
  
  header[buffer[1] - 1] = 0x51;
  
  memcpy(buffer + 3, header, buffer[1]);
  buffer[3 + buffer[1]] = request;
  
  return 3 + buffer[1] + buffer[2];
}

uint8_t setActionNdef(uint8_t *buffer, uint8_t action) {
  buffer[0] = 0xD4;
  buffer[1] = sizeof(header);
  buffer[2] = 0x01;
  
  header[buffer[1] - 1] = 0x41;
  
  memcpy(buffer + 3, header, buffer[1]);
  buffer[3 + buffer[1]] = action;
  
  return 3 + buffer[1] + buffer[2];
}

void print_ndef(const uint8_t *buffer, uint8_t length) {
  NdefMessage message = NdefMessage(buffer, length);
  message.print();
}
