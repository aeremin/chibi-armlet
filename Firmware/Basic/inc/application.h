/*
 * application.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "kl_lib_L15x.h"
#include "sequences.h"
#include "Dose.h"
#include "RxTable.h"

#if 1 // ==== Timings ====
#define TM_DOSE_INCREASE_MS 999
#define TM_DOSE_SAVE_MS     2007
#define TM_PILL_CHECK_MS    504    // Check if pill connected every TM_PILL_CHECK
#define TM_MEASUREMENT_MS   5004
#endif

// ========= Device types =========
#define DEVTYPE_UMVOS
//#define DEVTYPE_LUSTRA
//#define DEVTYPE_DETECTOR
//#define DEVTYPE_PILLPROG
//#define DEVTYPE_TUNER

// Sensitivity Constants, percent [1...1000]. Feel if RxLevel > SnsConst.
#define RLVL_NEVER              10000
#define RLVL_2M                 800     // 0...4m
#define RLVL_4M                 700     // 1...20m
#define RLVL_10M                600
#define RLVL_50M                1

#define RLVL_DETECTOR_RX        RLVL_4M // LED on Field will lit if rlevel is higher

// ==== Indication constants ====
#define BATTERY_DISCHARGED_ADC  1485    // 1200 mV

#if 1 // ==== Eeprom ====
// Addresses
#define EE_DEVICE_ID_ADDR       0
#define EE_DOSETOP_ADDR         (sizeof(uint32_t))  // ID is uint32_t
#endif

#if 1 // ==== Pill ====
#define PILL_TYPEID_SET_ID      0x0001
#define PILL_TYPEID_CURE        0x0009
#define PILL_TYPEID_SET_DOSETOP 0x0011

struct Pill_t {
    uint32_t TypeID;
    union {
        // Cure
        struct {
            uint32_t ChargeCnt;
            uint32_t Value;
        } __attribute__ ((__packed__));
        uint32_t DoseTop;
    };
} __attribute__ ((__packed__));
#define PILL_SZ     sizeof(Pill_t)
#endif // Pill

// ==== Application class ====
class App_t {
private:
    Pill_t Pill;
    uint8_t ISetID(uint32_t NewID);
    Eeprom_t EE;
#ifdef DEVTYPE_UMVOS
    Dose_t Dose;
    void SaveDoseTop() { EE.Write32(EE_DOSETOP_ADDR, Dose.Consts.Top); }
#endif
public:
    uint32_t ID;
    Thread *PThd;
#ifdef DEVTYPE_UMVOS
    RxTable_t RxTable;
#endif
    void Init();
    void DetectorFound(int32_t RssiPercent);
    // Events
    void OnPillConnect();
    void OnUartCmd(Cmd_t *PCmd);
#ifdef DEVTYPE_UMVOS
    void OnBatteryMeasured();
    void OnDoseIncTime();
    void OnRxTableReady();
#endif
};

extern App_t App;

#endif /* APPLICATION_H_ */
