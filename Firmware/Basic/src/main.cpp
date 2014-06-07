/*
 * File:   main.cpp
 * Author: Kreyl
 * Project: Armlet2South
 *
 * Created on Feb 05, 2013, 20:27
 */

#include "kl_lib_L15x.h"
#include "ch.h"
#include "hal.h"
#include "clocking_L1xx.h"
#include "indication.h"
#include "colors_sounds.h"
#include "pill_mgr.h"
#include "cmd_uart.h"
#include "application.h"
#include "radio_lvl1.h"
#include "eestore.h"
#include "evt_mask.h"
#include "adc15x.h"

#if 1 // ============================ Timers ===================================
void TmrUartRxCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_UART_RX_POLL);
    chVTSetI(&App.TmrUartRx, MS2ST(UART_RX_POLLING_MS), TmrUartRxCallback, nullptr);
    chSysUnlockFromIsr();
}
// Pill check
void TmrPillCheckCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_PILL_CHECK);
    chVTSetI(&App.TmrPillCheck, MS2ST(T_PILL_CHECK_MS), TmrPillCheckCallback, nullptr);
    chSysUnlockFromIsr();
}

void TmrDoseSaveCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_DOSE_STORE);
    chVTSetI(&App.TmrDoseSave, MS2ST(T_DOSE_SAVE_MS), TmrDoseSaveCallback, nullptr);
    chSysUnlockFromIsr();
}

void TmrMeasureCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_MEASURE_TIME);
    chVTSetI(&App.TmrMeasure, MS2ST(T_MEASUREMENT_MS), TmrMeasureCallback, nullptr);
    chSysUnlockFromIsr();
}

// Universal callback
void TmrGeneralCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, (eventmask_t)p);
    chSysUnlockFromIsr();
}
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V2);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();
    // ==== Init Hard & Soft ====
    Uart.Init(115200);
    Uart.Printf("\rRst\r\n");
    Indication.Init();
    PillMgr.Init();

    App.Init();
    App.PThd = chThdSelf();
    Radio.Init();

    // Battery measurement
    PinSetupAnalog(GPIOA, 0);
    Adc.InitHardware();
    Adc.PThreadToSignal = chThdSelf();

    PinSetupOut(GPIOC, 13, omPushPull, pudNone);
//    PinSet(GPIOC, 13);

    // Common Timers
    chSysLock();
    chVTSetI(&App.TmrUartRx,    MS2ST(UART_RX_POLLING_MS), TmrUartRxCallback, nullptr);
    chVTSetI(&App.TmrPillCheck, MS2ST(T_PILL_CHECK_MS),    TmrPillCheckCallback, nullptr);
    chVTSetI(&App.TmrMeasure,   MS2ST(99),                 TmrMeasureCallback, nullptr); // Start soon
    chSysUnlock();

    // Event-generating framework
    bool PillWasConnected = false;
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
        // ==== Uart cmd ====
        if(EvtMsk & EVTMSK_UART_RX_POLL) Uart.PollRx(); // Check if new cmd received

        // ==== Check pill ====
        if(EvtMsk & EVTMSK_PILL_CHECK) {
            bool IsNowConnected = (PillMgr.CheckIfConnected(PILL_I2C_ADDR) == OK);
            if(IsNowConnected and !PillWasConnected) {  // OnConnect
                PillWasConnected = true;
                App.OnPillConnect();
            }
            else if(!IsNowConnected and PillWasConnected) PillWasConnected = false;
        } // if EVTMSK_PILL_CHECK
        if(EvtMsk & EVTMSK_PROLONGED_PILL) App.OnProlongedPill();

        // ==== Dose ====
        if(EvtMsk & EVTMSK_DOSE_STORE) App.SaveDose();

        // ==== Measure battery ====
        if(EvtMsk & EVTMSK_MEASURE_TIME) Adc.StartMeasurement();
        if(EvtMsk & EVTMSK_MEASUREMENT_DONE) {
            uint32_t AdcRslt = Adc.GetResult(0);
//            Uart.Printf("Adc=%u\r", AdcRslt);
            Indication.BatteryState = (AdcRslt >= BATTERY_DISCHARGED_ADC)? bsGood : bsBad;
        }

        // ==== EMP ====
        if(EvtMsk & EVTMSK_KEY_POLL)      App.Grenade.OnKeyPoll();
        if(EvtMsk & EVTMSK_RADIATION_END) App.Grenade.OnRadiationEnd();

        // ==== Radio ====
        if(EvtMsk & EVTMSK_RX_TABLE_READY) App.OnRxTableReady();
    } // while true
}
