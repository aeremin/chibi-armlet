/*
 * real_time.h
 *
 *  Created on: 07 февр. 2014 г.
 *      Author: r.leonov
 */

#ifndef REAL_TIME_H_
#define REAL_TIME_H_

#include "kl_lib_L15x.h"
#include "cmd_uart.h"

#define SYNCHRO_TIMEOUT         4096
#define RTC_INIT_MASK           ((uint32_t)0xFFFFFFFF)
#define RTC_RSF_MASK            ((uint32_t)0xFFFFFF5F)

enum RTUMode_t {
    rtumHw, rtumFw, rtumNone
};

struct TimeContanier_t {
private:
public:
    union {
        uint8_t TimeBuf[3];
        struct {
            uint8_t HH;
            uint8_t MM;
            uint8_t SS;
        };
    }__attribute__ ((__packed__));
    void ResetTime()    { HH = 0; MM = 0; SS = 0; }
    void SetTime(uint32_t Ams) {
        HH = (Ams/1000)/3600;
        MM = ((Ams/1000)/60) - (HH*60);
        SS = (Ams/1000) - ((HH*3600) + (MM*60));
    }
};



class HwTime_t {
private:
    void rccRTCEnable()             { RCC->CSR |= RCC_CSR_RTCEN;     }
    void rccPwrEnable()             { rccEnableAPB1(RCC_APB1ENR_PWREN, false); }
    void BkupRwrEnable()            { PWR->CR |= PWR_CR_DBP; }
    void DisableWriteProtection()   { RTC->WPR = 0xCA; RTC->WPR = 0x53; }
    void EnableWriteProtection()    { RTC->WPR = 0xFF; }
    uint8_t WaitConfiguration();

    void SetInit()                  { RTC->ISR = (uint32_t)RTC_INIT_MASK; }
    uint8_t EnterInit();
    void ExitInit()                 { RTC->ISR &= RTC_ISR_INIT; }
    void EnableBackup()             { PWR->CR |= PWR_CR_DBP; }
    void Reset()                    { RCC->CSR |= RCC_CSR_RTCRST; RCC->CSR &= ~RCC_CSR_RTCRST; }

    void PutTimeMS(uint32_t Ams) {
//            RTU.Time.SetTime(Ams);
//            uint32_t tmp = 0;
//            tmp = (RTU.Time.HH << 16) | (RTU.Time.MM << 8) | (RTU.Time.SS);
//            Uart.Printf("RTC: put %X\r", tmp);
//            RTC->TR = tmp;
        }
public:
    void SetTime(uint8_t Ahh, uint8_t Amm, uint8_t Ass);
    void SetTimeMS(uint32_t Ams);
    uint8_t SetTimeBCD(uint8_t Ahh, uint8_t Amm, uint8_t Ass);
    uint32_t GetTimeMS();
    void Init();
};
extern HwTime_t HwTime;

class FwTime_t {
private:
    uint32_t absMS;
public:
    VirtualTimer RTUTmt;
    void TimInc() { absMS++; }
    uint32_t GetMs()  { return absMS; }
    void SetMs(uint32_t NValue) { absMS = NValue; }
    uint8_t SetTime(uint8_t HH, uint8_t MM, uint8_t SS);
    void Init();
};
extern FwTime_t FwTime;

class Time_t {
private:
    TimeContanier_t Time, *PTime;
    RTUMode_t TimeMode;
public:
    Time_t() : PTime(&Time), TimeMode(rtumNone) {}
    void Init(RTUMode_t Mode);
};


extern Time_t RTU;

#endif /* REAL_TIME_H_ */
