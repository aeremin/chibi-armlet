/*
 * application.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "kl_lib_L15x.h"
#include "Dose.h"
#include "RxTable.h"
#include "pill.h"
#include "ee.h"
#include "pill_mgr.h"

#if 1 // ==== Timings ====
#define T_DOSE_INCREASE_MS 999
#define T_DOSE_SAVE_MS     2007
#define T_PILL_CHECK_MS    360    // Check if pill connected every TM_PILL_CHECK
#define T_PROLONGED_PILL_MS 999
#define T_MEASUREMENT_MS   5004
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

// ==== Indication constants ====
#define BATTERY_DISCHARGED_ADC  1485    // 1200 mV

#define DO_DOSE_SAVE            FALSE

// ==== Application class ====
class App_t {
private:
    // Pill
    Pill_t Pill;
    DataToWrite_t Data2Wr;  // for pill flasher
    inline uint8_t IPillHandlerPillFlasher();
    inline uint8_t IPillHandlerUmvos();
    void SaveDoseToPill() {
        if(Dose.Value != Pill.DoseAfter)
            PillMgr.Write(PILL_I2C_ADDR, (PILL_START_ADDR + PILL_DOSEAFTER_ADDR), &Dose.Value, 4);
    }

    // Common
    uint8_t ISetID(uint32_t NewID);
    uint8_t ISetType(uint8_t AType);
    Dose_t Dose;
    uint32_t LastTime;
public:
    uint32_t ID;
    DeviceType_t Type;
    Thread *PThd;
    // Timers
    VirtualTimer TmrUartRx, TmrPillCheck, TmrDoseSave, TmrMeasurement, TmrProlongedPill;
    // Radio & damage
    RxTable_t RxTable;
    int32_t Damage;
    void SaveDose() { if(Dose.SaveValue() != OK) Uart.Printf("Dose Store Fail\r"); }
    void Init();
    // Events
    void OnPillConnect();
    void OnUartCmd(Cmd_t *PCmd);
    void OnBatteryMeasured();
    void OnRxTableReady();
    uint8_t OnProlongedPill();
    friend class Indication_t;
};

extern App_t App;

#endif /* APPLICATION_H_ */
