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
#include "cmd_uart.h"
#include "radio_lvl1.h"
#include "application.h"

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V8);
    Clk.UpdateFreqValues();

    // ==== Init OS ====
    halInit();
    chSysInit();
    // ==== Init Hard & Soft ====
//    if(ClkResult) Uart.Printf("Clock failure\r");
    Uart.Init(115200);
    Uart.Printf("CheckTX AHB=%u; APB1=%u; APB2=%u\r", Clk.AHBFreqHz, Clk.APB1FreqHz, Clk.APB2FreqHz);
    PinSetupOut(GPIOB, 5, omPushPull);
    PinSet(GPIOB, 5);

    Radio.Init();

    while(1) {
        chThdSleep(TIME_INFINITE); // Forever
    } // while
}
