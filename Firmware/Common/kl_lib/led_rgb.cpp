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
    Led.IStartSequenceI((const LedChunk_t*)p);
    chSysUnlockFromIsr();
}

void LedRGB_t::Init() {
    R.Init();
    G.Init();
    B.Init();
    // Initial value
    SetColor(clBlack);
}

void LedRGB_t::IStartSequenceI(const LedChunk_t *PLedChunk) {
    // Reset timer
    if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
    // Process the sequence
    while(PLedChunk != nullptr) {
        switch(PLedChunk->ChunkSort) {
            case csSetColor:
                if(PLedChunk->Time_ms == 0) {   // If smooth time is zero,
                    SetColor(PLedChunk->Color); // set color now,
                    PLedChunk++;                // and goto next chunk
                }
                else {
                    ICurrColor.Adjust(&PLedChunk->Color);
                    SetColor(ICurrColor);
                    // Which color is most different?
                    uint8_t ChValue = ICurrColor.MostDifferentChannel(&PLedChunk->Color);
                    // Check if equal
                    if(ChValue == 0) PLedChunk++; // Adjustment completed
                    else { // Calculate time to next adjustment


                    }
                }
                break;

            case csWait: // Start timer, pointing to next chunk
                chVTSetI(&ITmr, MS2ST(PLedChunk->Time_ms), LedTmrCallback, (void*)(PLedChunk+1));
                return;
                break;

            case csJump:
                PLedChunk = IPStartChunk + PLedChunk->ChunkToJumpTo;
                break;

            case csEnd: return; break;
        } // switch
    } // while
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
