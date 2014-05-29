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
    // ==== Quick status indication ====
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

int32_t Indication_t::IDoseDetectorMobile() {
    if(App.Damage > 0) {
        int32_t r = rand() % (DMG_SND_MAX - 1);
        int32_t DmgSnd = (((DMG_SND_MAX - DMG_SND_BCKGND) * (App.Damage - 1)) / (DMG_MAX - 1)) + DMG_SND_BCKGND;
//        Uart.Printf("%d; %d\r", Damage, DmgSnd);
        if(r < DmgSnd) TIM2->CR1 = TIM_CR1_CEN | TIM_CR1_OPM;
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
    if(LongState == lsSetType) {
        Led.SetColor(DeviceColor[App.Type]);   // Blink with correct color
        if(App.Type != dtDetectorMobile) Beeper.Beep(BeepBeep);
        LongState = lsNone;
        chThdSleepMilliseconds(T_SETTYPE_BLINK_MS);
        Led.SetColor(clBlack);
        chThdSleepMilliseconds(T_SETTYPE_BLINK_MS);
    }
    else {
        int32_t SleepInterval;
        switch(App.Type) {
            case dtUmvos: SleepInterval = Indication.ITaskUmvos(); break;
            default: SleepInterval = 999; break;
        } // switch
        WaitEvent(SleepInterval);
    }
}

void Indication_t::WaitEvent(uint32_t t_ms) {
    uint32_t EvtMsk = chEvtWaitAnyTimeout(ALL_EVENTS, t_ms);
    // ==== Pill ====
    if(EvtMsk & EVTMSK_PILL_CHECK) {
        if(PillState != piNone) {
            Led.SetColor(BBPill[PillState].Color1);
            Beeper.Beep(BBPill[PillState].PBeep);
            chThdSleepMilliseconds(BBPill[PillState].Time1_ms);
            Led.SetColor(BBPill[PillState].Color2);
            chThdSleepMilliseconds(BBPill[PillState].Time2_ms);
            PillState = piNone;
        }
    } // pill
}

//    // ==== Battery ====

void Indication_t::Init() {
    Led.Init();
    // Thread
    PThd = chThdCreateStatic(waIndicationThread, sizeof(waIndicationThread), LOWPRIO, (tfunc_t)IndicationThread, NULL);
}

void Indication_t::Reset() {
    // Deinit
    chSysLock();
    if(chVTIsArmedI(&TmrClick)) chVTResetI(&TmrClick);
    chSysUnlock();
    Beeper.Init();
    // Init depending on type
    uint32_t tmp1;
    switch(App.Type) {
        case dtDetectorMobile:
            // Setup TIM2 to click sound
            rccEnableTIM2(FALSE);
            PinSetupAlterFunc(GPIOB, 3, omPushPull, pudNone, AF1);
            TIM2->CR1 = TIM_CR1_CEN | TIM_CR1_OPM;
            TIM2->CR2 = 0;
            TIM2->ARR = 22;
            TIM2->CCMR1 |= (0b111 << 12);
            TIM2->CCER  |= TIM_CCER_CC2E;
            tmp1 = TIM2->ARR * 1000;
            tmp1 = Clk.APB1FreqHz / tmp1;
            if(tmp1 != 0) tmp1--;
            TIM2->PSC = (uint16_t)tmp1;
            TIM2->CCR2 = 20;
            break;
        default: break;
    }
    // Indicate type
    LongState = lsSetType;
}
#endif

void Indication_t::HealthRenew() {
    // Play sound
//    switch(App.Dose.State) {
//        case hsDeath: Beeper.Beep(BeepDeath); break;
//        case hsRedFast:
//            Led.StartBlink(LedRedFast);
//            Beeper.Beep(BeepRedFast);
//            break;
//        case hsRedSlow:
//            Led.StartBlink(LedRedSlow);
//            Beeper.Beep(BeepBeep);
//            break;
//        case hsYellow:
//            Led.StartBlink(LedYellow);
//            Beeper.Beep(BeepBeep);
//            break;
//        case hsGreen:
//            Led.StartBlink(LedGreen);
//            Beeper.Beep(BeepBeep);
//            break;
//    }

}

#if 1 // ==== Pill ====
//Beeper.Beep(BeepPillOk);
//Led.StartBlink(LedPillCureOk);
//}
//else {
//Beeper.Beep(BeepPillBad);
//Led.StartBlink(LedPillBad);
//}
//chThdSleepMilliseconds(1008);   // Let indication to complete
#endif

#if 1 // ==== Pelengator ====
//void FOnPelengatorLost() { chEvtSignalI(App.PThd, EVTMSK_PELENG_LOST); }
//
//void Indication_t::PelengReceived() {
//    Led.StartBlink(&TypeColorTblPeleng[Type], FOnPelengatorLost);
//}
//
//void Indication_t::PelengLost() {
//    switch(Type) {
//        case dtUmvos: Dose.RenewIndication(); break;
//        case dtDetectorFixed:
//            break;
//        case dtEmpMech:
//            break;
//        default: break;
//    }
//}
#endif


