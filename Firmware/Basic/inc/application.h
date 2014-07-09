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
#include "cmd_uart.h"

#if 1 // ==== Timings ====
#define TM_DOSE_INCREASE_MS 999
#define TM_DOSE_SAVE_MS     2007
#define TM_PILL_CHECK_MS    360    // Check if pill connected every TM_PILL_CHECK
#define TM_MEASUREMENT_MS   5004
#define TM_CLICK            18      // for Detector
#endif

// ========= Device types =========
enum DeviceType_t {
    dtNothing = 0,
    dtUmvos = 1,
    dtLustraClean = 2,
    dtLustraWeak = 3,
    dtLustraStrong = 4,
    dtLustraLethal = 5,
    dtDetectorMobile = 6,
    dtDetectorFixed = 7,
    dtEmpMech = 8,
    dtEmpGrenade = 9,
    dtPelengator = 10,
    dtPillFlasher = 11
};

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
enum PillType_t {
    ptSetID = 1,
    ptCure = 9,
    ptDrug = 10,
};
#define PILL_TYPEID_SET_ID      1
#define PILL_TYPEID_CURE        9
#define PILL_TYPEID_DRUG        10
#define PILL_TYPEID_PANACEA     11
#define PILL_TYPEID_AUTODOC     12
#define PILL_TYPEID_SET_DOSETOP 18
#define PILL_TYPEID_DIAGNOSTIC  27
#define PILL_TYPEID_EMP_BREAKER 31
#define PILL_TYPEID_EMP_REPAIR  32
#define PILL_TYPEID_EMP_CHARGE  33

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

// Data to save in EE and to write to pill
struct DataToWrite_t {
    uint32_t Sz32;
    int32_t Data[PILL_SZ32];
};
#endif // Pill

#if 1 // ==== Eeprom ====
// Addresses
#define EE_DEVICE_ID_ADDR       0
#define EE_DEVICE_TYPE_ADDR     4
#define EE_DOSETOP_ADDR         8  // ID is uint32_t
#define EE_REPDATA_ADDR         12
#endif

#define DO_DOSE_SAVE            FALSE

// ==== Application class ====
class App_t {
private:
    Pill_t Pill;
    DataToWrite_t Data2Wr;  // for pill flasher
    uint8_t ISetID(uint32_t NewID);
    uint8_t ISetType(uint8_t AType);
    Dose_t Dose;
    Eeprom_t EE;
    void SaveDoseTop() { EE.Write32(EE_DOSETOP_ADDR, Dose.Consts.Top); }
    uint32_t LastTime;
public:
    uint32_t ID;
    DeviceType_t Type;
    Thread *PThd;
    // Timers
    VirtualTimer TmrUartRx, TmrPillCheck, TmrDoseSave, TmrMeasurement, TmrClick;
    // Radio & damage
    uint32_t Damage;
    void SaveDose() { if(Dose.Save() != OK) Uart.Printf("Dose Store Fail\r"); }
    void Init();
    void DetectorFound(int32_t RssiPercent);
    // Events
    void OnPillConnect();
    void OnUartCmd(Cmd_t *PCmd);
    void OnBatteryMeasured();
    void OnRxTableReady();
    void OnClick();
};

extern App_t App;

#endif /* APPLICATION_H_ */
