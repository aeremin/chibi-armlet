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
#include "mesh_lvl.h"
#include "real_time.h"

static inline void Init();

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V8);
//    Clk.SetupFlashLatency(24);  // Setup Flash Latency for clock in MHz
//    Clk.SetupBusDividers(ahbDiv1, apbDiv1, apbDiv1);
    Clk.UpdateFreqValues();

    // ==== Init OS ====
    halInit();
    chSysInit();
    // ==== Init Hard & Soft ====
    Init();
//    if(ClkResult) Uart.Printf("Clock failure\r");

    while(1) {
        //chThdSleep(TIME_INFINITE);
//        Uart.Printf("%X:%X:%X\r", (((RTC->TR) & 0x7F0000) >>16), ((RTC->TR & 0x7F00) >>8), (RTC->TR & 0x7F));
//        Uart.Printf("%u\r", FwTime.GetMs());
        chThdSleepMilliseconds(999);
        Vibro.Flinch(Brr);
    } // while
}

void Init() {
    RTU.Init(rtumFw);   // Init Timer By second

    Uart.Init(57600);
    Uart.Printf("ChibiArmlet AHB=%u; APB1=%u; APB2=%u\r", Clk.AHBFreqHz, Clk.APB1FreqHz, Clk.APB2FreqHz);
    Led.Init();
//    Beeper.Init();
//    Beeper.Beep(BeepBeep);
//    Vibro.Init();
//    Vibro.Flinch(BrrBrr);
//    PillMgr.Init();

//    Radio.Init(SELF_MESH_ID);
//    Mesh.Init(SELF_MESH_ID);

    App.Init();
}
