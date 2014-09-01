/*
 * power_led.cpp
 *
 *  Created on: 01 сент. 2014 г.
 *      Author: r.leonov
 */

#include "power_led.h"


PowerLed_t PowerLed;

static void ILedTmrCallback(void *p) { PowerLed.ITmrCallback(); }

void PowerLed_t::Init() {
    // GPIO
    PinSetupAlterFunc(GPIOB, 0, omPushPull, pudNone, AF2);
    PinSetupAlterFunc(GPIOB, 1, omPushPull, pudNone, AF2);
    PinSetupAlterFunc(GPIOB, 5, omPushPull, pudNone, AF2);
    // Timer
    LED_TIM_RCC_EN();
    LED_TIM->CR1 = TIM_CR1_CEN; // Enable timer, set clk division to 0, AutoReload not buffered
    LED_TIM->CR2 = 0;
    LED_TIM->PSC = 0;
    LED_TIM->ARR = LED_INTENCITY_TOP;
    // Channels
    LED_TIM->CCMR1 = (0b110 << 12);
    LED_TIM->CCMR2 = (0b110 << 4) | (0b110 << 12);
    LED_TIM->CCER  = TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    // Initial values
    PCCR = &POWER_LED_GREEN_CCR;
}

void PowerLed_t::FadeIn() {
    chSysLock();
    if(CurrentIntencity == LED_INTENCITY_TOP) Lss = lssIdle;
    else {
        Lss = lssFadeIn;
        DesiredIntencity = LED_INTENCITY_TOP;
        // Start timer if not started yet
        if(!chVTIsArmedI(&ITmr)) {
            On();
            uint32_t Delay = IValue2Delay(CurrentIntencity);
            chVTSetI(&ITmr, MS2ST(Delay), ILedTmrCallback, nullptr);
        }
    }
    chSysUnlock();
}

void PowerLed_t::FadeOut() {
    chSysLock();
    if(CurrentIntencity == 0) Lss = lssIdle;
    else {
        Lss = lssFadeOut;
        DesiredIntencity = 0;
        // Start timer if not started yet
        if(!chVTIsArmedI(&ITmr)) {
            On();
            uint32_t Delay = IValue2Delay(CurrentIntencity);
            chVTSetI(&ITmr, MS2ST(Delay), ILedTmrCallback, nullptr);
        }
    }
    chSysUnlock();
}

void PowerLed_t::Gleam() {
    chSysLock();
    // Check bounds
    if(CurrentIntencity == 0) {
        Lss = lssGleamUp;
        DesiredIntencity = LED_GLEAM_INTENCITY_TOP;
    }
    else {
        Lss = lssGleamDown;
        DesiredIntencity = 0;
    }
    // Start timer if not started yet
    if(!chVTIsArmedI(&ITmr)) {
        On();
        uint32_t Delay = IValue2Delay(CurrentIntencity);
        chVTSetI(&ITmr, MS2ST(Delay), ILedTmrCallback, nullptr);
    }
    chSysUnlock();
}

void PowerLed_t::Blink(const uint32_t ABlinkDelay) {
    if(BlinkIsOngoing) return;
    chSysLock();
    BlinkIsOngoing = true;
    Off();
    if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
    chVTSetI(&ITmr, MS2ST(ABlinkDelay), ILedTmrCallback, nullptr);
    chSysUnlock();
}

void PowerLed_t::ITmrCallback() {
    // Process Blink
    if(BlinkIsOngoing) {
        BlinkIsOngoing = false;
        On();
    }
    // Process smooth
    if((Lss == lssFadeIn) or (Lss == lssFadeOut)) {
        if(CurrentIntencity == DesiredIntencity) Lss = lssIdle; // End of smooth occured somewhere else
        else {
            if(CurrentIntencity < DesiredIntencity) CurrentIntencity++;
            else CurrentIntencity--;
            On();
            if(CurrentIntencity == DesiredIntencity) Lss = lssIdle;
            else {
                uint32_t Delay = IValue2Delay(CurrentIntencity);
                chVTSetI(&ITmr, MS2ST(Delay), ILedTmrCallback, nullptr);
            }
        } // if ==
    } // if fade
    else if((Lss == lssGleamDown) or (Lss == lssGleamUp) or (Lss == lssGleamPause)) {
        uint32_t Delay;
        if(CurrentIntencity == DesiredIntencity) {
            switch(Lss) {
                case lssGleamDown:
                    Lss = lssGleamPause;    // Led is off, just wait
                    Delay = LED_GLEAM_PAUSE_MS;
                    break;

                case lssGleamUp:
                    Lss = lssGleamDown;
                    Delay = LED_GLEAM_PAUSE_MS;
                    DesiredIntencity = 0;
                    break;

                default: // Pause -> Up
                    Lss = lssGleamUp;
                    CurrentIntencity = 1;
                    Delay = IValue2Delay(CurrentIntencity);
                    DesiredIntencity = LED_GLEAM_INTENCITY_TOP;
                    break;
            } // switch
        }
        else {
            if(CurrentIntencity < DesiredIntencity) CurrentIntencity++;
            else CurrentIntencity--;
            Delay = IValue2Delay(CurrentIntencity);
        }
        On();
        chVTSetI(&ITmr, MS2ST(Delay), ILedTmrCallback, nullptr);
    } // if gleam up/down
}


