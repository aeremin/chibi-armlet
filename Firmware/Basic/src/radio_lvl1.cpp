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

void rLevel1_t::ITask() {
    uint8_t RxRslt = OK;
    uint32_t StartTime;
    int32_t Remainder;

    while(true) {
        // New cycle begins
        CC.Recalibrate();   // Recalibrate manually every cycle, as auto recalibration disabled
        // Transmit
        DBG1_SET();
        CC.TransmitSync(&PktTx);
        DBG1_CLR();

        // Listen
        StartTime = chTimeNow();
        Remainder = 270;
        while(Remainder > 0) {
            RxRslt = CC.ReceiveSync(Remainder, &PktRx);
            switch(RxRslt) {
                case TIMEOUT:
                    //Uart.Printf("TO\r");
                    break;
                case FAILURE:
                    Uart.Printf("Fl\r");
                    break;
                case OK:
                    //Uart.Printf("%d  %A\r", PktRx.RSSI, (uint8_t*)&PktRx, RPKT_LEN, ' ');
                    Uart.Printf("%d\r", PktRx.RSSI);
                    break;
                default: break;
            } // switch
            Remainder = (StartTime + 270) - chTimeNow();
        } // while remainder
    } // while true

//    uint32_t Dmg = 0;
    // Iterate channels
//    for(uint32_t n = CHANNEL_ZERO; n < (CHANNEL_ZERO + SLOW_EMANATOR_CNT - 1); n++) {
//        CC.SetChannel(n);
//        uint8_t Result = CC.ReceiveSync(RX_DURATION_SLOW_MS, &PktRx);
//        if(Result == OK) {
//            Uart.Printf("%A; Lvl=%d\r", (uint8_t*)&PktRx, RPKT_LEN, ' ', PktRx.RSSI);
//        } // id rslt ok
//    } // for
}
#endif

#if 1 // ============================
void rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
#endif
    // Init radioIC
    CC.Init();
    CC.SetTxPower(Pwr0dBm);
    CC.SetChannel(CHANNEL_ZERO);
    // Variables
    // Thread
    chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
