/*
 * application.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "kl_lib_L15x.h"

# if 1 // Uart Command Codes. See https://docs.google.com/document/d/1pGQf9CrQ016ObS0w7PhPLAy92MRPhdBriICflt1YGXA/edit
#define CMD_PING            0x01
#define CMD_SET_ID          0x10
#define CMD_GET_ID          0x11
#define CMD_SET_TYPE        0x16
#define CMD_GET_TYPE        0x17
#define CMD_PILL_STATE      0x30
#define CMD_PILL_WRITE      0x31
#define CMD_PILL_READ       0x32

#define RPL_ACK             0x90    // Acknowledge
#define RPL_GET_ID          0xA1
#define RPL_GET_TYPE        0xA7
#define RPL_PILL_STATE      0xC0
#define RPL_PILL_WRITE      0xC1
#define RPL_PILL_READ       0xC2
#endif

#if 1 // ==== Timings ====
#define TM_PILL_CHECK_MS        504     // Check if pill connected every TM_PILL_CHECK
#endif


#if 1 // ==== Pill ====
#define PILL_ID_CURE        0x09

#endif

enum DeviceType_t {
    dtNothing=0,
    dtFieldWeak=1, dtFieldNature=2, dtFieldStrong=3,
    dtXtraNormal=4, dtXtraWeak=5, dtUfo=6,
    dtDetector=7
};

#define EE_DEVICE_ID_ADDR       0
#define EE_DEVICE_TYPE_ADDR     4

class App_t {
private:
    void IPillHandler();
public:
    uint32_t ID;
    DeviceType_t Type;
    Thread *PThd;
    Eeprom_t EE;
    void Init();
    uint8_t SetType(uint8_t AType);
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* APPLICATION_H_ */
