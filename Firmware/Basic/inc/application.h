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
#define TM_CLICK            18      // for Detector
#endif

// ========= Device types =========
//#define DEVTYPE_UMVOS
//#define DEVTYPE_LUSTRA
//#define DEVTYPE_DETECTOR
#define DEVTYPE_PILLPROG
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

#if 1 // ==== Pill ====
#define PILL_TYPEID_SET_ID      1
#define PILL_TYPEID_CURE        9
#define PILL_TYPEID_DRUG        10
#define PILL_TYPEID_PANACEA     11
#define PILL_TYPEID_SET_DOSETOP 18
#define PILL_TYPEID_DIAGNOSTIC  27

struct Pill_t {
    int32_t TypeID;
    union {
        int32_t DeviceID;
        int32_t DoseAfter;  // Contains dose value after pill application
    };
    union {
        // Cure / drug
        struct {
            int32_t ChargeCnt;
            int32_t Value;
        } __attribute__ ((__packed__));
        int32_t DoseTop;
    };
} __attribute__ ((__packed__));
#define PILL_SZ     sizeof(Pill_t)
#define PILL_SZ32   (sizeof(Pill_t) / sizeof(int32_t))

// data to save in EE and to write to pill
struct DataToWrite_t {
    uint32_t Sz32;
    int32_t Data[PILL_SZ32];
};

#endif // Pill

#if 1 // ==== Eeprom ====
// Addresses
#define EE_DEVICE_ID_ADDR       0
#define EE_DOSETOP_ADDR         4  // ID is uint32_t
#define EE_REPDATA_ADDR         8
#endif

// ==== Application class ====
class App_t {
private:
    Pill_t Pill;
    uint8_t ISetID(uint32_t NewID);
    Eeprom_t EE;
#ifdef DEVTYPE_UMVOS
    Dose_t Dose;
    void SaveDoseTop() { EE.Write32(EE_DOSETOP_ADDR, Dose.Consts.Top); }
    uint32_t LastTime;
#endif
#ifdef DEVTYPE_PILLPROG
    DataToWrite_t Data2Wr;
#endif
public:
    uint32_t ID;
    Thread *PThd;
#if defined DEVTYPE_UMVOS || defined DEVTYPE_DETECTOR
    RxTable_t RxTable;
    uint32_t Damage;
#endif
#ifdef DEVTYPE_UMVOS
    void SaveDose() { if(Dose.Save() != OK) Uart.Printf("Dose Store Fail\r"); }
#endif
    void Init();
    void DetectorFound(int32_t RssiPercent);
    // Events
    void OnPillConnect();
    void OnUartCmd(Cmd_t *PCmd);
#if defined DEVTYPE_UMVOS || defined DEVTYPE_DETECTOR
    void OnBatteryMeasured();
    void OnRxTableReady();
#endif
#ifdef DEVTYPE_DETECTOR
    void OnClick();
#endif
};

extern App_t App;

#endif /* APPLICATION_H_ */
