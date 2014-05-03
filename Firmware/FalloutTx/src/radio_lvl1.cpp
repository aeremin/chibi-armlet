/*
 * radio_lvl1.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#include "radio_lvl1.h"
#include "evt_mask.h"
#include "cc1101.h"
#include "cmd_uart.h"

#define DBG_PINS

#ifdef DBG_PINS
#define DBG_GPIO1   GPIOC
#define DBG_PIN1    15
#define DBG1_SET()  PinSet(DBG_GPIO1, DBG_PIN1)
#define DBG1_CLR()  PinClear(DBG_GPIO1, DBG_PIN1)
#endif

rLevel1_t Radio;
rPkt_t PktTx;

#if 1 // ================================ Task =================================
static WORKING_AREA(warLvl1Thread, 256);
__attribute__((noreturn))
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    while(true) {
        DBG1_SET();
        CC.TransmitSync(&PktTx);
        DBG1_CLR();
    }
}
#endif

#if 1 // ============================
void rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
#endif
    // Init RadioPkt
    PktTx.Type = 18;
    PktTx.Check[0] = CHECK_0;
    PktTx.Check[1] = CHECK_1;
    PktTx.Check[2] = CHECK_2;

    // Init radioIC
    CC.Init();
    CC.SetTxPower(CC_PwrMinus10dBm);
    CC.SetChannel(0);
    CC.SetPktSize(RPKT_LEN);
    // Thread
    chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
