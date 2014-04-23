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
#define TM_PILL_CHECK_MS    504    // Check if pill connected every TM_PILL_CHECK
#define TM_MEASUREMENT_MS   5004
// Delay between demonstration
#define TM_DEMO_COMMON_MS   2007
#define TM_DEMO_DETECTOR_MS 702
#endif


#if 1 // ==== Pill ====
#define PILL_TYPEID_SETTYPE 0x0020

struct Pill_t {
    uint16_t TypeID;
    uint8_t DeviceType;
} __attribute__ ((__packed__));
#define PILL_SZ     sizeof(Pill_t)

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
#define RLVL_2M                 800     // 0...4m
#define RLVL_4M                 700     // 1...20m
#define RLVL_10M                600
#define RLVL_50M                1

#define RLVL_DETECTOR_RX        RLVL_4M // LED on Field will lit if rlevel is higher

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
#define LVL_STEPS_N             4   // 0 is nothing, 1...3 is level of indication
#define BATTERY_DISCHARGED_ADC  1485    // 1200 mV

// ==== Eeprom addresses ====
#define EE_DEVICE_ID_ADDR       0
#define EE_DEVICE_TYPE_ADDR     4

struct TypeLvl_t {
    DeviceType_t Type = dtNothing;
    int32_t Level = 0;
    void Set(DeviceType_t AType, int32_t ALvl) {
        chSysLock();
        Type = AType;
        Level = ALvl;
        chSysUnlock();
    }
    bool IsNotEmpty() {
        bool r;
        chSysLock();
        r = (Type != dtNothing) and (Level >= 1) and (Level <= 4);
        chSysUnlock();
        return r;
    }
    void Clear() { Type = dtNothing; Level = 0; }
};

// ==== Application class ====
class App_t {
private:
    Pill_t Pill;
    void IPillHandler();
    inline void ITableHandler();
    inline void IDemonstrate();
    TypeLvl_t Demo; // What to demonstrate
public:
    uint32_t ID;
    DeviceType_t Type;
    Thread *PThd;
    Eeprom_t EE;
    RxTable_t RxTable;
    void Init();
    uint8_t SetType(uint8_t AType);
    void DetectorFound(int32_t RssiPercent);
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* APPLICATION_H_ */
