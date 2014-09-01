/*
 * power_led.h
 *
 *  Created on: 01 сент. 2014 г.
 *      Author: r.leonov
 */

#ifndef POWER_LED_H_
#define POWER_LED_H_

#include "kl_lib_L15x.h"
#include "cmd_uart.h"

#define LED_INTENCITY_TOP       1998
#define LED_GLEAM_INTENCITY_TOP 270
#define LED_GLEAM_PAUSE_MS      1440

#define LED_TIM                   TIM3
#define POWER_LED_RED_CCR         LED_TIM->CCR4
#define POWER_LED_GREEN_CCR       LED_TIM->CCR3
#define POWER_LED_BLUE_CCR        LED_TIM->CCR2
#define LED_TIM_RCC_EN()          rccEnableTIM3(FALSE)

enum LedColor_t {lcRed, lcBlue, lcGreen};
enum LedSmoothState_t {lssIdle, lssFadeIn, lssFadeOut, lssGleamUp, lssGleamDown, lssGleamPause};

class PowerLed_t {
private:
    VirtualTimer ITmr;
    bool BlinkIsOngoing;
    LedSmoothState_t Lss;
    uint32_t CurrentIntencity, DesiredIntencity;
    volatile uint32_t *PCCR;    // Current color
    void Color2PCCR(LedColor_t AColor) { PCCR = (AColor == lcRed)? &POWER_LED_RED_CCR : ((AColor == lcBlue)? &POWER_LED_BLUE_CCR : &POWER_LED_GREEN_CCR); }
    uint32_t IValue2Delay(uint16_t AValue) { return (uint32_t)((450 / (AValue+4)) + 1); }
public:
    void Init();
    void SetIntencity(uint32_t AIntencity) { CurrentIntencity = AIntencity; }
    void Off() { *PCCR = 0; }
    void On()  { *PCCR = CurrentIntencity; }
    void BlinkSmoothly();
    void FadeOut();
    void FadeIn();
    void Gleam();
    void Idle() { Lss = lssIdle; }
    void Blink(const uint32_t ABlinkDelay = 55);
    // Inner use
    void ITmrCallback();
};

extern PowerLed_t PowerLed;

#endif /* POWER_LED_H_ */
