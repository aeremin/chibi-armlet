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

// ==== LedRGB itself ====
class LedRGB_t {
private:
    const LedChunk_t *IPFirstChunk;
    VirtualTimer ITmr;

public:
    void Init();
    void SetColor(Color_t AColor) {
        R.Set(AColor.Red);
        G.Set(AColor.Green);
        B.Set(AColor.Blue);
    }
    void StartBlink(const LedChunk_t *PLedChunk) {
        chSysLock();
        IPFirstChunk = PLedChunk; // Save first chunk
        IStartBlinkI(PLedChunk);
        chSysUnlock();
    }
    void StopBlink() {
        chSysLock();
        if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
        SetColor(clBlack);
        chSysUnlock();
    }
    // Inner use
    void IStartBlinkI(const LedChunk_t *PLedChunk);
};

extern LedRGB_t Led;

#endif /* LED_RGB_H_ */
