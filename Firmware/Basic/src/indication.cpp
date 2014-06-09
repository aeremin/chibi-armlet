/*
 * indication.cpp
 *
 *  Created on: 28 мая 2014 г.
 *      Author: Kreyl
 */

#include "indication.h"
#include "application.h"

Indication_t Indication;

#if 1 // ==== Device-dependent indication ====
int32_t Indication_t::ITaskUmvos() {
    // Check if health state changed
    static HealthState_t OldState = hsDeath;
    if(App.Dose.State != OldState) {
        OldState = App.Dose.State;
        Beeper.Beep(BeepBeep);
    }
    // ==== Health state ====
    const BlinkBeep_t *pbb = &BBHealth[App.Dose.State];
    Led.SetColor(pbb->Color1);
    if(pbb->PBeep != nullptr) Beeper.Beep(pbb->PBeep);
    chThdSleepMilliseconds(pbb->Time1_ms);

    // ==== Battery ====
    if(BatteryState == bsBad) {
        Led.SetColor(clBlack);
        chThdSleepMilliseconds(54);
        Led.SetColor(pbb->Color1);
        chThdSleepMilliseconds(T_BATTERY_BLINK_MS);
    }

    // ==== Autodoc ====
    if(App.AutodocActive) {
        Led.SetColor(BB_ADInProgress.Color2);
        chThdSleepMilliseconds(BB_ADInProgress.Time2_ms);
        Led.SetColor(BB_ADInProgress.Color1);
        chThdSleepMilliseconds(BB_ADInProgress.Time1_ms);
        Led.SetColor(BB_ADInProgress.Color2);
        chThdSleepMilliseconds(BB_ADInProgress.Time2_ms);
    }
    // Proceed with dose demonstration
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

int32_t Indication_t::ITaskDetectorFixed() {
    static DamageLevel_t DmgLvlOld = dlDirty;
    // Calculate current damage level
    DamageLevel_t DmgLvl = dlClear;
    if(App.Damage == 1) DmgLvl = dlFon;
    else if(App.Damage > 1) DmgLvl = dlDirty;
    // Detect if level changed
    if(DmgLvlOld != DmgLvl) {
        DmgLvlOld = DmgLvl;
        Beeper.Beep(BeepBeep);
    }
    // ==== Indicate depending on received damage ====
    const BlinkBeep_t *pbb = &BB_DmgLevel[DmgLvl];
    Led.SetColor(pbb->Color1);
    chThdSleepMilliseconds(pbb->Time1_ms);
    // ==== Battery ====
    if(BatteryState == bsBad) {
        Led.SetColor(clBlack);
        chThdSleepMilliseconds(54);
        Led.SetColor(pbb->Color1);
        chThdSleepMilliseconds(T_BATTERY_BLINK_MS);
    }
    // Proceed with dose demonstration
    Led.SetColor(pbb->Color2);
    return pbb->Time2_ms;
}

int32_t Indication_t::ITaskGrenade() {
    const BlinkBeep_t *pbb = &BB_Grenade[App.Grenade.State];
    Led.SetColor(pbb->Color1);
    if(pbb->PBeep != nullptr) Beeper.Beep(pbb->PBeep);
    chThdSleepMilliseconds(pbb->Time1_ms);
    // ==== Battery ====
    if(BatteryState == bsBad) {
        Led.SetColor(clBlack);
        chThdSleepMilliseconds(54);
        Led.SetColor(pbb->Color1);
        chThdSleepMilliseconds(T_BATTERY_BLINK_MS);
    }
    // Proceed
    Led.SetColor(pbb->Color2);
    return pbb->Time2_ms;
}

int32_t Indication_t::ITaskEmpMech() {
    // Check if changed
    static MechState_t StateOld = msOperational;
    MechState_t StateNow = App.Mech.GetState();
    if(StateOld != StateNow) {
        StateOld = StateNow;
        if(StateNow == msBroken) Beeper.Beep(&BeepMechBroken);
        else if(StateNow == msOperational) Beeper.Beep(&BeepMechRepaired);
    }

    const BlinkBeep_t *pbb = &BB_EmpMech[App.Mech.GetState()];
    Led.SetColor(pbb->Color1);
    chThdSleepMilliseconds(pbb->Time1_ms);
    // ==== Battery ====
    if(BatteryState == bsBad) {
        Led.SetColor(clBlack);
        chThdSleepMilliseconds(54);
        Led.SetColor(pbb->Color1);
        chThdSleepMilliseconds(T_BATTERY_BLINK_MS);
    }
    // Proceed
    Led.SetColor(pbb->Color2);
    return pbb->Time2_ms;
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
        case dtUmvos:          SleepInterval = ITaskUmvos();          break;
        case dtDetectorMobile: SleepInterval = ITaskDetectorMobile(); break;
        case dtDetectorFixed:  SleepInterval = ITaskDetectorFixed();  break;
        case dtEmpGrenade:     SleepInterval = ITaskGrenade();        break;
        case dtEmpMech:        SleepInterval = ITaskEmpMech();        break;
        case dtPelengator:
            Led.SetColor(DeviceColor[MaxSignalLvlDevType]);
            SleepInterval = 999;
            break;

        case dtLustraClean:
        case dtLustraWeak:
        case dtLustraStrong:
        case dtLustraLethal:
            SleepInterval = 4005;
            break;

        default: SleepInterval = 999; break;
    } // switch

    // ==== Peleng received by some device ====
    if(IPelengReceived) {
        IPelengReceived = false;
        Color_t Clr = DeviceColor[App.Type];
        if(ANY_OF_4(App.Type, dtLustraClean, dtLustraWeak, dtLustraStrong, dtLustraLethal)) {
            if(BatteryState == bsBad) Clr = clRed;
        }
        Led.SetColor(Clr);
    }

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
    if((EvtMsk & EVTMSK_PILL_CHECK) and (PillState != piNone)) {
        DoBeepBlink(&BBPill[PillState]);
        PillState = piNone;
    } // pill

    // ==== Lustra's Bad ID ====
    if(EvtMsk & EVTMSK_LUSTRA_BAD_ID) DoBeepBlink(&BB_BadID);

    // ==== Autodoc ====
    if(EvtMsk & EVTMSK_AUTODOC_COMPLETED) DoBeepBlink(&BB_ADCompleted);
    if(EvtMsk & EVTMSK_AUTODOC_EXHAUSTED) DoBeepBlink(&BB_ADExhausted);
#endif // Event
    // Finally, fade LED
    Led.SetColor(clBlack);
    if(App.Type != dtDetectorMobile) chThdSleepMilliseconds(T_SHORT_BLINK_MS);
}

void Indication_t::DoBeepBlink(const BlinkBeep_t *Pbb) {
    Led.SetColor(Pbb->Color1);
    if(Pbb->PBeep != nullptr) Beeper.Beep(Pbb->PBeep);
    chThdSleepMilliseconds(Pbb->Time1_ms);
    Led.SetColor(Pbb->Color2);
    chThdSleepMilliseconds(Pbb->Time2_ms);
}

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
