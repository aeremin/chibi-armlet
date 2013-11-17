/*
 * radio_lvl1.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#include "radio_lvl1.h"
#include "evt_mask.h"
#include "application.h"
#include "cc1101.h"

rLevel1_t Radio;

#if 1 // ================================ Task =================================
static WORKING_AREA(warLvl1Thread, 256);
__attribute__((noreturn))
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    while(true) Radio.ITask();
}

void rLevel1_t::ITask() {
    // Slow cycle begins
    // TODO: calibrate CC
//    uint32_t Dmg = 0;
    // Iterate channels
    for(uint32_t n = CHANNEL_ZERO; n < (CHANNEL_ZERO + SLOW_EMANATOR_CNT - 1); n++) {
        CC.SetChannel(n);
        uint8_t Result = CC.ReceiveSync(RX_DURATION_SLOW_MS, &PktRx);
        if(Result == OK) {
            Uart.Printf("%A; Lvl=%d\r", (uint8_t*)&PktRx, RPKT_LEN, ' ', PktRx.RSSI);
        } // id rslt ok
    } // for
}
#endif

#if 1 // ============================
void rLevel1_t::Init() {
    // Init radioIC
    CC.Init();
    //CC.SetTxPower(PwrPlus12dBm);
    //CC.SetTxPower(PwrMinus6dBm);
    // Variables
    // Thread
    chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), NORMALPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
