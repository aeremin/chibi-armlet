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
#include "pill.h"

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
    RxTable_t RxTable;
    uint32_t Damage;
    void SaveDose() { if(Dose.Save() != OK) Uart.Printf("Dose Store Fail\r"); }
    void Init();
    void DetectorFound(int32_t RssiPercent);
    // Events
    void OnPillConnect();
    void OnPillDisconnect();
    void OnUartCmd(Cmd_t *PCmd);
    void OnBatteryMeasured();
    void OnRxTableReady();
    void OnClick();
};

extern App_t App;

#endif /* APPLICATION_H_ */
