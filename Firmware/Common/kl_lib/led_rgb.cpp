/*
 * led_rgb.cpp
 *
 *  Created on: 31 рту. 2014 у.
 *      Author: Kreyl
 */

#include "led_rgb.h"

LedRGB_t Led;

// Timer callback
static void LedTmrCallback(void *p) {
    chSysLockFromIsr();
    Led.IStartBlinkI((const LedChunk_t*)p);
    chSysUnlockFromIsr();
}


void LedRGB_t::Init() {
    R.Init();
    G.Init();
    B.Init();
    // Initial value
    SetColor(clGreen);
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
    switch(PLedChunk->ChunkSort) {
        case ckNormal: PCh = PLedChunk + 1; break;
        case ckStop:                        break;
        case ckJump: PCh = IPFirstChunk;  break;
    }
    chVTSetI(&ITmr, MS2ST(PLedChunk->Time_ms), LedTmrCallback, (void*)PCh); // Start timer
}


#if 1 // ============================= LED Channel =============================
void LedChnl_t::Init() const {
    // ==== GPIO setup ====
    if(PTimer == TIM2) PinSetupAlterFunc(PGpio, Pin, omPushPull, pudNone, AF1);
    else if(PTimer == TIM3 or PTimer == TIM4) PinSetupAlterFunc(PGpio, Pin, omPushPull, pudNone, AF2);
    else PinSetupAlterFunc(PGpio, Pin, omPushPull, pudNone, AF3);

    // ==== Timer setup ====
    if     (PTimer == TIM2)  { rccEnableTIM2(FALSE); }
    else if(PTimer == TIM3)  { rccEnableTIM3(FALSE); }
    else if(PTimer == TIM4)  { rccEnableTIM4(FALSE); }
    else if(PTimer == TIM9)  { rccEnableTIM9(FALSE); }
    else if(PTimer == TIM10) { rccEnableAPB2(RCC_APB2ENR_TIM10EN, FALSE); }
    else if(PTimer == TIM11) { rccEnableAPB2(RCC_APB2ENR_TIM11EN, FALSE); }

    PTimer->CR1 = TIM_CR1_CEN; // Enable timer, set clk division to 0, AutoReload not buffered
    PTimer->CR2 = 0;
    PTimer->ARR = LED_TOP_VALUE;

    // ==== Timer's channel ====
#if LED_INVERTED_PWM
#define PwmMode 0b111
#else
#define PwmMode 0b110
#endif
    switch(TmrChnl) {
        case 1:
            PTimer->CCMR1 |= (PwmMode << 4);
            PTimer->CCER  |= TIM_CCER_CC1E;
            break;
        case 2:
            PTimer->CCMR1 |= (PwmMode << 12);
            PTimer->CCER  |= TIM_CCER_CC2E;
            break;
        case 3:
            PTimer->CCMR2 |= (PwmMode << 4);
            PTimer->CCER  |= TIM_CCER_CC3E;
            break;
        case 4:
            PTimer->CCMR2 |= (PwmMode << 12);
            PTimer->CCER  |= TIM_CCER_CC4E;
            break;
        default: break;
    }
}

#endif
