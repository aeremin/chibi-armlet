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
#include "pill.h"
#include "cmd_uart.h"
#include "application.h"

#include "eestore.h"

EEStore_t EE;

static inline void Init();

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V8);
    //Clk.SetupFlashLatency(24);  // Setup Flash Latency for clock in MHz
//    Clk.SetupBusDividers(ahbDiv1, apbDiv1, apbDiv1);
    Clk.UpdateFreqValues();

    // ==== Init OS ====
    halInit();
    chSysInit();
    // ==== Init Hard & Soft ====
    Init();
//    if(ClkResult) Uart.Printf("Clock failure\r");

    uint32_t ob = 0;
    uint8_t r;
    EEChunk32_t *P;
    P = EE_PTR;
    for(uint8_t i=0; i<4; i++) {
        Uart.Printf("  %u %X %u\r", P->Counter, P->Sign, P->Data);
        P++;
    }

    r = EE.Get(&ob);
    Uart.Printf("%u, %u\r", r, ob);
    ob+=9;
    r = EE.Put(&ob);
    Uart.Printf("%u\r", r);
    ob = 0;
    r = EE.Get(&ob);
    Uart.Printf("%u, %u\r", r, ob);

    P = EE_PTR;
    for(uint8_t i=0; i<4; i++) {
        Uart.Printf("  %u %X %u\r", P->Counter, P->Sign, P->Data);
        P++;
    }

    while(1) {
        //chThdSleepMilliseconds(999);
        chSysLock();
        chThdSleepS(TIME_INFINITE); // Forever
        chSysUnlock();
//        Led.StopBlink();
    } // while
}

void Init() {
    Uart.Init(115200);
    Led.Init();
//    Led.SetColor(clGreen);
//    Beeper.Init();
//    Beeper.Beep(BeepBeep);
//    PillInit();
//    App.Init();
//    Led.StartBlink(ShortGreen);
    Uart.Printf("ChibiArmlet AHB=%u; APB1=%u; APB2=%u\r", Clk.AHBFreqHz, Clk.APB1FreqHz, Clk.APB2FreqHz);
}
