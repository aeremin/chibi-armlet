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
#include "peripheral.h"
#include "sequences.h"
#include "pill_mgr.h"
#include "cmd_uart.h"
#include "application.h"
#include "radio_lvl1.h"
#include "eestore.h"
#include "evt_mask.h"

#if 1 // ============================ Timers ===================================
static VirtualTimer ITmrPillCheck, ITmrMeasurement;
void TmrPillCheckCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_PILL_CHECK);
    chVTSetI(&ITmrPillCheck, MS2ST(TM_PILL_CHECK_MS), TmrPillCheckCallback, nullptr);
    chSysUnlockFromIsr();
}
void TmrMeasurementCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_MEASURE_TIME);
    chVTSetI(&ITmrMeasurement, MS2ST(TM_MEASUREMENT_MS), TmrMeasurementCallback, nullptr);
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
    Led.Init();
    Led.SetColor(clWhite);
    chThdSleepMilliseconds(540);
    Led.SetColor(clBlack);

    Beeper.Init();
    Beeper.Beep(BeepBeep);
    PillMgr.Init();
    Radio.Init();

    App.Init();
    Uart.Printf("TestingRX AHB=%u; ID=%u\r", Clk.AHBFreqHz, App.ID);
    App.PThd = chThdSelf();
    // Battery measurement
//    PinSetupAnalog(GPIOA, 0);
//    Adc.InitHardware();
//    Adc.PThreadToSignal = PThd;
    // ==== Init event generating framework ====
    // Timers init
    chSysLock();
    chVTSetI(&ITmrMeasurement, MS2ST(TM_MEASUREMENT_MS), TmrMeasurementCallback, nullptr);
    chVTSetI(&ITmrPillCheck, MS2ST(TM_PILL_CHECK_MS),    TmrPillCheckCallback, nullptr);
    chSysUnlock();

    // Event-generating framework
    bool PillConnected = false;
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
        // ==== Check pill ====
        if(EvtMsk & EVTMSK_PILL_CHECK) {
            // Check if new connection occured
            if(PillMgr.CheckIfConnected(PILL_I2C_ADDR) == OK) {
                if(!PillConnected) {
                    PillConnected = true;
                    App.OnPillConnect();
                }
            }
            else PillConnected = false;
        } // if EVTMSK_PILL_CHECK

        // ==== Measure battery ====
//        if(EvtMsk & EVTMSK_MEASURE_TIME) Adc.StartMeasurement();
//        if(EvtMsk & EVTMSK_MEASUREMENT_DONE) {
//            uint32_t AdcRslt = Adc.GetResult(BATTERY_CHNL);
//            Uart.Printf("Adc=%u\r", AdcRslt);
//            // Blink Red if discharged
//            if(AdcRslt < BATTERY_DISCHARGED_ADC) Led.StartBlink(LedDischarged);
//        }

    } // while true
}
