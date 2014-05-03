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

# if 1 // Uart Command Codes. See https://docs.google.com/document/d/1pGQf9CrQ016ObS0w7PhPLAy92MRPhdBriICflt1YGXA/edit
#define CMD_PING            0x01
#define CMD_SET_ID          0x10
#define CMD_GET_ID          0x11
#define CMD_PILL_STATE      0x30
#define CMD_PILL_WRITE      0x31
#define CMD_PILL_READ       0x32

#define RPL_ACK             0x90    // Acknowledge
#define RPL_GET_ID          0xA1
#define RPL_PILL_STATE      0xC0
#define RPL_PILL_WRITE      0xC1
#define RPL_PILL_READ       0xC2

#define UART_RPL_BUF_SZ     36
#endif

#if 1 // ==== Timings ====
#define TM_PILL_CHECK_MS    504    // Check if pill connected every TM_PILL_CHECK
#define TM_MEASUREMENT_MS   5004
#endif

// ========= Device types =========
//#define DEVTYPE_UMVOS
//#define DEVTYPE_DETECTOR
//#define DEVTYPE_LUSTRA
//#define DEVTYPE_GUL
//#define DEVTYPE_PILLPROG
//#define DEVTYPE_TUNER

#if 1 // ==== Pill ====
#define PILL_TYPEID_SET_ID  0x0001

struct Pill_t {
    uint16_t TypeID;
    union {
        uint16_t DeviceID;
    };
} __attribute__ ((__packed__));
#define PILL_SZ     sizeof(Pill_t)

#endif // Pill

// Sensitivity Constants, percent [1...1000]. Feel if RxLevel > SnsConst.
#define RLVL_NEVER              10000
#define RLVL_2M                 800     // 0...4m
#define RLVL_4M                 700     // 1...20m
#define RLVL_10M                600
#define RLVL_50M                1

#define RLVL_DETECTOR_RX        RLVL_4M // LED on Field will lit if rlevel is higher

// ==== Indication constants ====
#define BATTERY_DISCHARGED_ADC  1485    // 1200 mV

// ==== Eeprom addresses ====
#define EE_DEVICE_ID_ADDR       0

// ==== Application class ====
class App_t {
private:
    Pill_t Pill;
    uint8_t ISetID(uint32_t NewID);
    uint8_t UartRplBuf[UART_RPL_BUF_SZ];
public:
    uint32_t ID;
    Thread *PThd;
    Eeprom_t EE;
    void Init();
    void DetectorFound(int32_t RssiPercent);
    // Events
    void OnPillConnect();
    void OnBatteryMeasured();
    void OnUartCmd(uint8_t CmdCode, uint8_t *PData, uint32_t Length);
};

extern App_t App;

#endif /* APPLICATION_H_ */
