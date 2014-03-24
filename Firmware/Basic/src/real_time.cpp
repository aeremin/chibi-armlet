/*
 * real_time.cpp
 *
 *  Created on: 07 февр. 2014 г.
 *      Author: r.leonov
 */


#include "real_time.h"

Time_t RTU; // Real Time Unit, based on RTC

void Time_t::Init() {
    // Config TIM10 for LSI
//    RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
//    nvicEnableVector(TIM10_IRQn, CORTEX_PRIORITY_MASK(IRQ_PRIO_MEDIUM));
//    TIM10->PSC = 0;
//    TIM10->EGR = TIM_EGR_UG;
//    TIM10->OR = TIM_OR_TI1RMP_0; // LSI internal clock is connected to the TIM10_CH1
//    TIM10->CCMR1 = TIM_CCMR1_CC1S_0;    // Direct
//    TIM10->CCMR1 |= TIM_CCMR1_IC1PSC; // every 8 cycles
//    TIM10->CR1 |= TIM_CR1_CEN; // Enable
//    TIM10->SR = 0;
//    TIM10->DIER = TIM_DIER_CC1IE;
//
    Reset();
    rccPwrEnable();
    BkupRwrEnable();

    // LSI
    RCC->CSR |= RCC_CSR_LSION;
    while ((RCC->CSR & RCC_CSR_LSIRDY) == 0);
    rccRTCEnable();
    RCC->CSR |= RCC_CSR_RTCSEL_LSI;

    SetTime(10,15,00);
}

void Time_t::SetTime(uint8_t Ahh, uint8_t Amm, uint8_t Ass) {
    uint32_t Ams;
    Ams =   Ahh*3600*1000;
    Ams +=  Amm*60*1000;
    Ams +=  Ass*1000;

    DisableWriteProtection();
    /* Set Initialization mode */
    RTC->ISR = (uint32_t)RTC_INIT_MASK;
    if(EnterInit() == OK) {
        PutTimeMS(Ams);
        RTC->ISR &= (uint32_t)~RTC_ISR_INIT;
        EnableWriteProtection();
    }
}

void Time_t::SetTimeMS(uint32_t Ams) {
    DisableWriteProtection();
    /* Set Initialization mode */
    RTC->ISR = (uint32_t)RTC_INIT_MASK;
    if(EnterInit() == OK) {
        PutTimeMS(Ams);
        RTC->ISR &= (uint32_t)~RTC_ISR_INIT;
        EnableWriteProtection();
    }
}

void Time_t::SetTimeBCD(uint8_t Ahh, uint8_t Amm, uint8_t Ass) {
    DisableWriteProtection();
    /* Set Initialization mode */
    RTC->ISR = (uint32_t)RTC_INIT_MASK;
    if(EnterInit() == OK) {
        RTC->TR = (Ahh << 16) | (Amm << 8) | Ass;
        RTC->ISR &= (uint32_t)~RTC_ISR_INIT;
        EnableWriteProtection();
    }
}


uint8_t Time_t::WaitConfiguration() {
    uint8_t Status;
    uint32_t SynchroCounter = 0;
    DisableWriteProtection();
    do {
        if(RTC->ISR & RTC_ISR_RSF) {
            Status = OK;
            continue;
        }
        SynchroCounter++;
    } while(SynchroCounter < SYNCHRO_TIMEOUT);
    Status = FAILURE;
    EnableWriteProtection();
    return Status;
}

uint8_t Time_t::EnterInit() {
    uint32_t SynchroCounter = 0;
    if(RTC->ISR & RTC_ISR_INITF) return OK;  // if already in init mode
    RTC->ISR = RTC_ISR_INIT;
    do {
        if(RTC->ISR & RTC_ISR_INITF) return OK;
        SynchroCounter++;
    } while(SynchroCounter < SYNCHRO_TIMEOUT);
    return FAILURE;
}


