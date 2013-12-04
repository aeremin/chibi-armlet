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
#include "cmd_uart.h"

#include "peripheral.h"

#define DBG_PINS

#ifdef DBG_PINS
#define DBG_GPIO1   GPIOB
#define DBG_PIN1    14
#define DBG1_SET()  PinSet(DBG_GPIO1, DBG_PIN1)
#define DBG1_CLR()  PinClear(DBG_GPIO1, DBG_PIN1)
#endif


rLevel1_t Radio;

#if 1 // ================================ Task =================================
static WORKING_AREA(warLvl1Thread, 256);
__attribute__((noreturn))
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    while(true) Radio.ITask();
}

//#define TX
//#define LED_RX

void rLevel1_t::ITask() {
    while(true) {
//        Uart.Printf("c");
        // New cycle begins
        CC.Recalibrate();   // Recalibrate manually every cycle, as auto recalibration disabled

        uint32_t NaturalDmg = 1, RadioDmg = 0;
        // Iterate slow emanators
        for(uint8_t i=0; i<SLOW_EMANATOR_CNT; i++) {
            CC.SetChannel(CHANNEL_ZERO + i);
            uint8_t RxRslt = CC.ReceiveSync(27, &PktRx);
            if(RxRslt == OK) {
                int32_t prc = RSSI_DB2PERCENT(PktRx.RSSI);
//                Uart.Printf("%u\r", PktRx.ID);
                // "Clean zone" emanator
                if((prc >= PktRx.MaxLvl) and (PktRx.DmgMax == 0) and (PktRx.DmgMin == 0)) NaturalDmg = 0;
                // Ordinal emanator
                else {
//                    Uart.Printf("%d; %d\r", PktRx.RSSI, prc);
                    if(prc >= PktRx.MaxLvl) RadioDmg += PktRx.DmgMax;
                    else if(prc >= PktRx.MinLvl) {
                        int32_t DifDmg = PktRx.DmgMax - PktRx.DmgMin;
                        int32_t DifLvl = PktRx.MaxLvl - PktRx.MinLvl;
                        int32_t EmDmg = (prc * DifDmg + PktRx.DmgMax * DifLvl - PktRx.MaxLvl * DifDmg) / DifLvl;
//                        Uart.Printf("%d; %d\r", prc, EmDmg);
                        if(EmDmg > 0) RadioDmg += EmDmg;
                    }
                }
//                if(RadioDmg != 0) Uart.Printf("%d; %d\r", prc, RadioDmg);
            } // if ok
        } // for
        // Sleep until asked
        CC.Sleep();
        Damage = NaturalDmg + RadioDmg;
//        if(RadioDmg != 0) Uart.Printf("%d\r", RadioDmg);
//        Uart.Printf("%d\r", Damage);
        chThdSleepMilliseconds(45);
    } // while true
}
#endif

#if 1 // ============================
void rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
#endif
    // Init radioIC
    CC.Init();
    CC.SetTxPower(CC_Pwr0dBm);
    CC.SetChannel(CHANNEL_ZERO);
    // Variables
    Damage = 1;
    // Thread
    PThread = chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
