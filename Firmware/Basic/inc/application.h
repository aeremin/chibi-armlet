/*
 * application.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "kl_lib_L15x.h"
#include "RxTable.h"
#include "sequences.h"

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

// ==== Device Types ====
enum DeviceType_t {
    dtNothing=0,
    dtFieldWeak=1, dtFieldNature=2, dtFieldStrong=3,
    dtXtraNormal=4, dtXtraWeak=5, dtUfo=6,
    dtDetector=7
};
// Sensitivity Constants, percent [1...1000]. Feel if RxLevel > SnsConst.
#define RLVL_NEVER              10000
#define RLVL_2M                 700
#define RLVL_4M                 500
#define RLVL_50M                1

// ==== Table of sensitivity constants ====
// 8 types of device, 3 nypes of field
const int32_t SnsTable[8][3] = {
        {RLVL_NEVER, RLVL_NEVER, RLVL_NEVER}, // dtNothing
        {RLVL_NEVER, RLVL_NEVER, RLVL_NEVER}, // dtFieldWeak
        {RLVL_NEVER, RLVL_NEVER, RLVL_NEVER}, // dtFieldNature
        {RLVL_NEVER, RLVL_NEVER, RLVL_NEVER}, // dtFieldStrong

        {RLVL_2M,    RLVL_50M,   RLVL_4M   }, // dtXtraNormal
        {RLVL_NEVER, RLVL_NEVER, RLVL_2M   }, // dtXtraWeak
        {RLVL_NEVER, RLVL_50M,   RLVL_2M   }, // dtUfo

        {RLVL_NEVER, RLVL_NEVER, RLVL_NEVER}, // dtDetector
};

// ==== Indication constants ====
#define LVL_STEPS_N             4 // 0 is nothing, 1...3 is level of indication

// ==== Eeprom addresses ====
#define EE_DEVICE_ID_ADDR       0
#define EE_DEVICE_TYPE_ADDR     4

// ==== Application class ====
class App_t {
private:
    void IPillHandler();
    inline void ITableHandler();
    inline void IDemonstrate(int32_t Level, DeviceType_t AType);
public:
    uint32_t ID;
    DeviceType_t Type;
    Thread *PThd;
    Eeprom_t EE;
    RxTable_t RxTable;
    void Init();
    uint8_t SetType(uint8_t AType);
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* APPLICATION_H_ */
