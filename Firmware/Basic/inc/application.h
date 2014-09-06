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
#include "cmd_uart.h"
#include "mesh_params.h"

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
// ==== mist support ====
#define MIST_SUPPORT_CHIBI
#ifdef MIST_SUPPORT_CHIBI
//#define MIST_TRANSLATE_TIME_SEC 600
#define MIST_TRANSLATE_TIME_SEC 30
#endif

#if 1 // ==== Pill ====
#define PILL_TYPEID_SET_ID      1

struct Pill_t {
    int32_t TypeID;
    int32_t DeviceID;
} __attribute__ ((__packed__));
#define PILL_SZ     sizeof(Pill_t)
#define PILL_SZ32   (sizeof(Pill_t) / sizeof(int32_t))

#endif // Pill

#if 1 // ==== Eeprom ====
// Addresses
#define EE_DEVICE_ID_ADDR       0
#endif

// ==== Application class ====
class App_t {
private:
    Pill_t Pill;
    uint8_t ISetID(uint32_t NewID);
    Eeprom_t EE;
public:
    uint32_t ID;
    Thread *PThd;
    // Timers
    VirtualTimer TmrUartRx, TmrPillCheck, TmrMeasurement, TmrFade;
    // Radio & damage
    state_t CurrInfo;
    void Init();
    void CheckIfLightGreen();
    // Events
    void OnPillConnect();
    void OnUartCmd(Cmd_t *PCmd);
    void OnBatteryMeasured();
    void OnRxTableReady();
    void CallBlueLightStop(){};
    void CallBlueLightStart(){};
#ifdef MIST_SUPPORT_CHIBI
    int32_t mist_msec_ctr; //-1 - not active, else - sec from begining
    uint16_t reason_saved;
    state_t send_info;
#endif

};

extern App_t App;

#endif /* APPLICATION_H_ */
