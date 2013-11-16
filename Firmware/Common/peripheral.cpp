/*
 * peripheral.cpp
 *
 *  Created on: 18.01.2013
 *      Author: kreyl
 */

#include "ch.h"
#include "hal.h"
#include "clocking_L1xx.h"
#include "peripheral.h"

#if 1 // =============================== Beep ==================================
#define BEEP_TOP_VALUE   22
Beeper_t Beeper;
// Timer callback
void BeeperTmrCallback(void *p) {
    chSysLockFromIsr();
    Beeper.BeepI((const BeepChunk_t*)p);
    chSysUnlockFromIsr();
}

void Beeper_t::Init() {
    IPin.Init(GPIOB, 3, TIM2, 2, BEEP_TOP_VALUE);
}

void Beeper_t::BeepI(const BeepChunk_t *PSequence) {
    // Reset timer
    if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
    if(PSequence == nullptr) {
        IPin.Off();
        return;
    }
    // Set sound
    IPin.SetFreqHz(PSequence->Freq_Hz);
    IPin.Set(PSequence->Volume);
    // Proceed sequence, stop it or restart
    const BeepChunk_t *PCh = nullptr;
    switch(PSequence->ChunkKind) {
        case ckNormal: PCh = PSequence + 1; break;
        case ckStop:                        break;
        case ckRepeat: PCh = IPFirstChunk;  break;
    }
    // Start timer
    chVTSetI(&ITmr, MS2ST(PSequence->Time_ms), BeeperTmrCallback, (void*)PCh);
}

void Beeper_t::Shutdown() {
    PinSetupAnalog(GPIOD, 12);
}
#endif

#if 1 // ============================== LED RGB ================================
LedRGB_t Led;
// Timer callback
static void LedTmrCallback(void *p) {
    chSysLockFromIsr();
    Led.IStartBlinkI((const LedChunk_t*)p);
    chSysUnlockFromIsr();
}

void LedRGB_t::Init() {
    // ==== GPIO ====
    PinSetupAlterFunc(LED_GPIO, LED_P1, omPushPull, pudNone, LED_ALTERFUNC);
    PinSetupAlterFunc(LED_GPIO, LED_P2, omPushPull, pudNone, LED_ALTERFUNC);
    PinSetupAlterFunc(LED_GPIO, LED_P3, omPushPull, pudNone, LED_ALTERFUNC);
    // ==== Timer ====
    LED_RCC_EN();
    // ==== Timebase and general ====
    LED_TIM->CR1 = 0x01;       // Enable timer, set clk division to 0, AutoReload not buffered
    LED_TIM->CR2 = 0;          // Output Idle State
    LED_TIM->PSC = 0;          // No clock division
    LED_TIM->ARR = 255;        // Autoreload register: top value of PWM
    // ==== Outputs ====
    const uint16_t OutCmpBits = 0b01100000; // output, PWM1
    LED_TIM->CCMR1 = OutCmpBits << 8;       // CCR2
    LED_TIM->CCMR2 = (OutCmpBits << 8) | OutCmpBits;
    LED_TIM->CCER = TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    // Initial value
    SetColor(clBlack);
}

void LedRGB_t::IStartBlinkI(const LedChunk_t *PLedChunk) {
    // Reset timer
    if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
    if(PLedChunk == nullptr) {
        SetColor(clBlack);
        return;
    }
    // Process chunk
    SetColor(PLedChunk->Color);
    // Proceed sequence, stop it or restart
    const LedChunk_t *PCh = nullptr;
    switch(PLedChunk->ChunkKind) {
        case ckNormal: PCh = PLedChunk + 1; break;
        case ckStop:                        break;
        case ckRepeat: PCh = IPFirstChunk;  break;
    }
    chVTSetI(&ITmr, MS2ST(PLedChunk->Time_ms), LedTmrCallback, (void*)PCh); // Start timer
}
#endif
