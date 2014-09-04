/*
 * File:   main.cpp
 * Author: Kreyl
 * Project: ChibiArmlet-Atlantis
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
#include "mesh_lvl.h"
#include "RxTable.h"

#include "power_led.h"

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
    chVTSetI(&App.TmrPillCheck, MS2ST(TM_PILL_CHECK_MS), TmrPillCheckCallback, nullptr);
    chSysUnlockFromIsr();
}

void TmrMeasurementCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_MEASURE_TIME);
    chVTSetI(&App.TmrMeasurement, MS2ST(TM_MEASUREMENT_MS), TmrMeasurementCallback, nullptr);
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
//    Led.Init();
    PillMgr.Init();
    PowerLed.Init();
    PowerLed.Gleam();

    App.Init();
    App.PThd = chThdSelf();

    Radio.Init();
    Mesh.Init();
//     Battery measurement
//    PinSetupAnalog(GPIOA, 0);
//    Adc.InitHardware();
//    Adc.PThreadToSignal = PThd;
    Beeper.Init();
//    Beeper.Beep(BeepBeep);

    // Timers
    chSysLock();
    chVTSetI(&App.TmrUartRx,    MS2ST(UART_RX_POLLING_MS), TmrUartRxCallback, nullptr);
    chVTSetI(&App.TmrPillCheck, MS2ST(TM_PILL_CHECK_MS),   TmrPillCheckCallback, nullptr);
    chSysUnlock();

    // Event-generating framework
    bool PillConnected = false;
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
        // ==== Uart cmd ====
        if(EvtMsk & EVTMSK_UART_RX_POLL) Uart.PollRx(); // Check if new cmd received

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

        // ==== Radio ====
        if(EvtMsk & EVTMSK_SENS_TABLE_READY) App.OnRxTableReady();
    } // while true

}
