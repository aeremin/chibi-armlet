/*
 * indication.cpp
 *
 *  Created on: 28 мая 2014 г.
 *      Author: Kreyl
 */

#include "indication.h"
#include "application.h"

Indication_t Indication;

#if 1 // =========================== Timers ====================================
// ==== Detector's click ====
//void TmrClickCallback(void *p) {
//    if(App.Damage > 0) {
//        int32_t r = rand() % (DMG_SND_MAX - 1);
//        int32_t DmgSnd = (((DMG_SND_MAX - DMG_SND_BCKGND) * (App.Damage - 1)) / (DMG_MAX - 1)) + DMG_SND_BCKGND;
////        Uart.Printf("%d; %d\r", Damage, DmgSnd);
//        if(r < DmgSnd) TIM2->CR1 = TIM_CR1_CEN | TIM_CR1_OPM;
//    }
//    // Restart timer
//    chSysLockFromIsr();
//    chVTSetI(&Indication.TmrClick, MS2ST(TM_CLICK_MS), TmrClickCallback, nullptr);
//    chSysUnlockFromIsr();
//}
#endif

#if 1 // ==== Dose indication ====
int32_t Indication_t::ITaskUmvos() {
    // Check if status changed
    static HealthState_t OldState = hsDeath;
    if(App.Dose.State != OldState) {
        OldState = App.Dose.State;
        Beeper.Beep(BeepBeep);
    }
    // Demonstrate dose status
    const BlinkBeep_t *pbb = &BBHealth[App.Dose.State];
    Led.SetColor(pbb->Color1);
    if(pbb->PBeep != nullptr) Beeper.Beep(pbb->PBeep);
    chThdSleepMilliseconds(pbb->Time1_ms);
    Led.SetColor(pbb->Color2);
    return pbb->Time2_ms;
}

int32_t Indication_t::ITaskDetectorMobile() {
    if(App.Damage > 0) {
        int32_t r = rand() % (DMG_SND_MAX - 1);
        int32_t DmgSnd = (((DMG_SND_MAX - DMG_SND_BCKGND) * (App.Damage - 1)) / (DMG_MAX - 1)) + DMG_SND_BCKGND;
//        Uart.Printf("%d; %d\r", Damage, DmgSnd);
        if(r < DmgSnd) Beeper.IPin.Enable();
    }
    return 18;
}
#endif

#if 1 // ===================== Thread & Task ===================================
static WORKING_AREA(waIndicationThread, 128);
static void IndicationThread(void *arg) {
    chRegSetThreadName("Indication");
    while(true) Indication.ITask();
}

void Indication_t::ITask() {
    // Indication depends on device type. Every function returns required sleep interval until next call.
    int32_t SleepInterval;
    switch(App.Type) {
        case dtUmvos:          SleepInterval = Indication.ITaskUmvos(); break;
        case dtDetectorMobile: SleepInterval = Indication.ITaskDetectorMobile(); break;
        default: SleepInterval = 999; break;
    } // switch

#if 1 // ==== Wait Event ====
    uint32_t EvtMsk = chEvtWaitAnyTimeout(ALL_EVENTS, SleepInterval);
    // ==== Type changed ====
    if(EvtMsk & EVTMSK_TYPE_CHANGED) {
        Led.SetColor(DeviceColor[App.Type]);   // Blink with correct color
        if(App.Type != dtDetectorMobile) Beeper.Beep(BeepBeep);
        chThdSleepMilliseconds(T_SETTYPE_BLINK_MS);
        Led.SetColor(clBlack);
        chThdSleepMilliseconds(T_SETTYPE_BLINK_MS);
    }

    // ==== Pill ====
    if(EvtMsk & EVTMSK_PILL_CHECK) {
        if(PillState != piNone) {
            const BlinkBeep_t *p = &BBPill[PillState];
            PillState = piNone;
            Led.SetColor(p->Color1);
            Beeper.Beep(p->PBeep);
            chThdSleepMilliseconds(p->Time1_ms);
            Led.SetColor(p->Color2);
            chThdSleepMilliseconds(p->Time2_ms);
        }
    } // pill

    // ==== Pelengator ====
    if(EvtMsk & EVTMSK_PELENG_FOUND) {
        Led.SetColor(DeviceColor[App.Type]);
        chThdSleepMilliseconds(T_PELENG_BLINK_MS);
        Led.SetColor(clBlack);
    }
#endif // Event
}
    // ==== Battery ====

void Indication_t::Init() {
    Led.Init();
    Beeper.Init();
    // Thread
    PThd = chThdCreateStatic(waIndicationThread, sizeof(waIndicationThread), LOWPRIO, (tfunc_t)IndicationThread, NULL);
}

void Indication_t::ProcessTypeChange() {
    // Setup beeper to beep or to click
    if(App.Type == dtDetectorMobile) {
        TIM2->CR1 = TIM_CR1_OPM;
        TIM2->ARR = 22;
        TIM2->CCMR1 = (0b111 << 12);
        TIM2->CCER = TIM_CCER_CC2E;
        uint32_t tmp = TIM2->ARR * 1000;
        tmp = Clk.APB1FreqHz / tmp;
        if(tmp != 0) tmp--;
        TIM2->PSC = (uint16_t)tmp;
        TIM2->CCR2 = 20;
    }
    else Beeper.Init();

    chEvtSignal(PThd, EVTMSK_TYPE_CHANGED);
}
#endif
