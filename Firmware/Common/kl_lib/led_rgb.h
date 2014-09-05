/*
 * led_rgb.h
 *
 *  Created on: 31 рту. 2014 у.
 *      Author: Kreyl
 */

#ifndef LED_RGB_H_
#define LED_RGB_H_

#include "hal.h"
#include "kl_lib_L15x.h"
#include "color.h"
#include "sequences.h"

// ==== LED's ports, pins and Tmr ====
struct LedChnl_t {
    GPIO_TypeDef *PGpio;
    uint16_t Pin;
    TIM_TypeDef *PTimer;
    uint32_t TmrChnl;
    void Set(const uint8_t AValue) const { *(uint32_t*)(&PTimer->CCR1 + TmrChnl - 1) = AValue; }    // CCR[N] = AValue
    void Init() const;
};

// Port, pin and timer settings. Edit this properly.
#define LED_TOP_VALUE       255
#define LED_INVERTED_PWM    FALSE
const LedChnl_t
    R = {GPIOB, 0, TIM3, 4},
    G = {GPIOB, 1, TIM3, 3},
    B = {GPIOB, 5, TIM3, 2};

// ==== Smoothening ====
/* TimeToWaitBeforeNextAdjustment = SmoothVar / (N+4) + 1, where N - current LED brightness.
 * SmoothVar depends on DesiredSetupTime and CurrentBrightness (==N):
 * SmoothVar = (DesiredSetupTime - LED_TOP_VALUE) * 1000 / HarmonicNumber[N]
 * Initially, SmoothVar==-1 to demonstrate necessity of recalculation */
extern const uint16_t LedHarmonicNumber[]; // Here, Harmonic numbers are sums of 1000/(N+4).

// ==== LedRGB itself ====
class LedRGB_t {
private:
    const LedChunk_t *IPStartChunk;
    int32_t SmoothVar;
    uint32_t ICalcDelay(int32_t CurrentBrightness) { return (uint32_t)((SmoothVar / (CurrentBrightness+4)) + 1); }
    void ICalcSmoothVar(int32_t CurrentBrightness, int32_t DesiredSetupTime) {
        SmoothVar = (DesiredSetupTime < LED_TOP_VALUE)? 0 : ((DesiredSetupTime - LED_TOP_VALUE) * 1000) / LedHarmonicNumber[CurrentBrightness];
    }
    VirtualTimer ITmr;
    Color_t ICurrColor;
    uint8_t *PMostDifferentChannel; // Pointer to one of ICurrentColor channels
    void ISetCurrent() {
        R.Set(ICurrColor.Red);
        G.Set(ICurrColor.Green);
        B.Set(ICurrColor.Blue);
    }
public:
    void Init();
    void SetColor(Color_t AColor) {
        R.Set(AColor.Red);
        G.Set(AColor.Green);
        B.Set(AColor.Blue);
        ICurrColor = AColor;
    }
    void StartSequence(const LedChunk_t *PLedChunk) {
        chSysLock();
        IPStartChunk = PLedChunk;   // Save first chunk
        SmoothVar = -1;             // Demonstrate necessity of recalculation
        IStartSequenceI(PLedChunk);
        chSysUnlock();
    }
    void Stop() {
        chSysLock();
        if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
        SetColor(clBlack);
        chSysUnlock();
    }
    // Inner use
    void IStartSequenceI(const LedChunk_t *PLedChunk);
};

extern LedRGB_t Led;

#endif /* LED_RGB_H_ */
