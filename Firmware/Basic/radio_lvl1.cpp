/*
 * radio_lvl1.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#include <main.h>
#include "radio_lvl1.h"
#include "evt_mask.h"
#include "cc1101.h"
#include "uart.h"
// For test purposes
#include "led.h"
extern LedRGB_t Led;

#define DBG_PINS

#ifdef DBG_PINS
#define DBG_GPIO1   GPIOC
#define DBG_PIN1    15
#define DBG1_SET()  PinSet(DBG_GPIO1, DBG_PIN1)
#define DBG1_CLR()  PinClear(DBG_GPIO1, DBG_PIN1)
#endif

rLevel1_t Radio;

#if 1 // ================================ Task =================================
static WORKING_AREA(warLvl1Thread, 256);
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    while(true) Radio.ITask();
}

void rLevel1_t::ITask() {
    // Check if ID changed
    if((uint8_t)App.ID != Chnl) {
        Chnl = (uint8_t)App.ID;
        CC.SetChannel(Chnl);
    }
    // Transmit
    DBG1_SET();
    CC.TransmitSync(&Pkt);
    DBG1_CLR();
}
#endif // task

#if 1 // ============================
uint8_t rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
#endif
    // Init radioIC
    if(CC.Init() == OK) {
        CC.SetTxPower(CC_PwrMinus6dBm);
        CC.SetPktSize(RPKT_LEN);
        CC.SetChannel(Chnl);
        Pkt.TestWord = TEST_WORD;
        // Thread
        chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
//        Uart.Printf("\rCC init OK");
        return OK;
    }
    else {
        Uart.Printf("\rCC init error");
        return FAILURE;
    }
}
#endif
