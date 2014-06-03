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
#define T_DOSE_INCREASE_MS  999
#define T_DOSE_SAVE_MS      2007
#define T_PILL_CHECK_MS     360  // Check if pill connected every TM_PILL_CHECK
#define T_PROLONGED_PILL_MS 999
#define T_MEASUREMENT_MS    5004 // Battery measurement
// EMP
#define T_KEY_POLL_MS       306
#define T_EMP_DURATION_MS   5004 // Duration of radiation
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

// ==== Emp ====
enum EmpState_t {empOperational=0, empBroken=1, empRepair=2, empDischarged=3, empCharging=4, empRadiating=5};
#define EMP_DEFAULT  empOperational
// ==== Key ====
#define KEY_GPIO    GPIOC
#define KEY_PIN     13

extern void TmrEmpCallback(void *p) __attribute__((unused));

class Emp_t {
private:
    VirtualTimer Tmr;
public:
    EmpState_t State;
    void KeyInit() { PinSetupIn(KEY_GPIO, KEY_PIN, pudPullUp); }
    bool KeyIsPressed() { return !PinIsSet(KEY_GPIO, KEY_PIN); }
    void SetTmrToPollKeyI() { chVTSetI(&Tmr, MS2ST(T_KEY_POLL_MS), TmrEmpCallback, (void*)EVTMSK_KEY_POLL_TIME); }
    void SetTmrToRadiationEnd() { chVTSet(&Tmr, MS2ST(T_EMP_DURATION_MS), TmrEmpCallback, (void*)EVTMSK_RADIATION_END); }
    void ResetTmrI() { if(chVTIsArmedI(&Tmr)) chVTResetI(&Tmr); }
    void Save() {
        if(EE.Read32(EE_EMP_STATE_ADDR) != (uint32_t)State)
            EE.Write32(EE_EMP_STATE_ADDR, (uint32_t)State);
    }
    void Load() {
        uint32_t tmp = EE.Read32(EE_EMP_STATE_ADDR);
        if(tmp > empCharging) State = EMP_DEFAULT;
        else State = (EmpState_t)tmp;
    }
    // Events
    void OnKeyPoll();
    void OnRadiationEnd();
};

// ==== Misc ====
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
    Emp_t Emp;
    // Timers
    VirtualTimer TmrUartRx, TmrPillCheck, TmrDoseSave, TmrMeasure, TmrProlongedPill;
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
