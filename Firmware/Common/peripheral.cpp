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
#define BEEP_TOP_VALUE   220 // 100% volume means on/off ratio 1/1
Beeper_t Beeper;
// Timer callback
void BeeperTmrCallback(void *p) {
    Beeper.BeepI((const BeepChunk_t*)p);
}

void Beeper_t::Init() {
    IPin.Init(GPIOB, 3, TIM2, 2, BEEP_TOP_VALUE);
}

void Beeper_t::BeepI(const BeepChunk_t *PSequence) {
    // Reset timer
    if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
    // Process chunk
    int8_t Volume = PSequence->VolumePercent;
    // Stop
    if(Volume == -1) {
        IPin.Off();
        return;
    }
    // Repeat
    else if(Volume == -2) {
        PSequence = IPFirstChunk;
        Volume = PSequence->VolumePercent;
        if(Volume < 0) {    // Nothing to play
            IPin.Off();
            return;
        }
    }

    if(Volume > 100) Volume = 100;  // Normalize volume
    IPin.SetFreqHz(PSequence->Freq_Hz);
    IPin.Set(Volume);
    // Start timer
    chVTSetI(&ITmr, MS2ST(PSequence->Time_ms), BeeperTmrCallback, (void*)(PSequence+1));
}

void BeeperTmrCallbackStop(void *p) {
    Beeper.IPin.Off();
}

void Beeper_t::Beep(uint32_t ms) {
    chVTReset(&ITmr);
    IPin.SetFreqHz(2000);
    IPin.Set(100);
    chVTSet(&ITmr, MS2ST(ms), BeeperTmrCallbackStop, NULL);
}

void Beeper_t::Shutdown() {
    PinSetupAnalog(GPIOD, 12);
}
#endif

#if 1 // ============================== LED RGB ================================
LedRGB_t Led;
// Timer callback
static void LedTmrCallback(void *p) {
    Led.IStartBlinkI((const LedChunk_t*)p);
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
    // Process chunk
    SetColor(PLedChunk->Color);
    // Restart sequence or proceed
    const LedChunk_t *PCh = (PLedChunk->ChunkKind == ckLast)? IPFirstChunk : (PLedChunk+1);
    chVTSetI(&ITmr, MS2ST(PLedChunk->Time_ms), LedTmrCallback, (void*)PCh); // Start timer
}
#endif
