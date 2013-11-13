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
Eeprom_t EE;
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

    uint32_t w = EE.Read32(0);
    Uart.Printf("%X\r", w);
    EE.Unlock();
    uint8_t r = EE.Write32(0, 0xDEADBEEF);
    EE.Lock();
    Uart.Printf("r: %u\r", r);
    w = EE.Read32(0);
    Uart.Printf("%X\r", w);

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
    Beeper.Init();
    Beeper.Beep(BeepBeep);
    PillInit();
    App.Init();
//    Led.StartBlink(ShortGreen);
    Uart.Printf("ChibiArmlet AHB=%u; APB1=%u; APB2=%u\r", Clk.AHBFreqHz, Clk.APB1FreqHz, Clk.APB2FreqHz);
}
