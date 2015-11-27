#ifndef __DEBUG_H__
#define __DEBUG_H__

// #define DEBUG
// #define DEBUGSNEP

#include "Arduino.h"

#ifdef DEBUG
#define DMSG(args...)       Serial.print(args)
#define DMSG_STR(str)       Serial.println(str)
#define DMSG_HEX(num)       Serial.print(' '); Serial.print(num, HEX)
#define DMSG_INT(num)       Serial.print(' '); Serial.print(num)
#else
#define DMSG(args...)
#define DMSG_STR(str)
#define DMSG_HEX(num)
#define DMSG_INT(num)
#endif

#ifdef DEBUGSNEP
#define DMS(args...)       Serial.print(args)
#define DMS_STR(str)       Serial.println(str)
#define DMS_HEX(num)       Serial.print(' '); Serial.print(num, HEX)
#define DMS_INT(num)       Serial.print(' '); Serial.print(num)
#else
#define DMS(args...)
#define DMS_STR(str)
#define DMS_HEX(num)
#define DMS_INT(num)
#endif

#endif
