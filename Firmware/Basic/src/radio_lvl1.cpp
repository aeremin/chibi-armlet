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

//#define DBG_PINS

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

//#define TX
void rLevel1_t::ITask() {
#ifdef TX
        // Transmit
        PktTx.ID = 4;
        PktTx.TestWord = 0xCa115ea1;
        CC.TransmitSync(&PktTx);
        chThdSleepMilliseconds(99);
#else
        Color_t Clr;
        int8_t Rssi;
        uint8_t RxRslt = CC.ReceiveSync(306, &PktRx, &Rssi);
        if(RxRslt == OK) {
            if(PktRx.TestWord == 0xCa115ea1) {
                Uart.Printf("%d\r", Rssi);
                Clr = clWhite;
                if     (Rssi < -100) Clr = clRed;
                else if(Rssi < -90) Clr = clYellow;
                else if(Rssi < -80) Clr = clGreen;
                else if(Rssi < -70) Clr = clCyan;
                else if(Rssi < -60) Clr = clBlue;
                else if(Rssi < -50) Clr = clMagenta;
                Led.SetColor(Clr);

                // Set 1 at PC15
                PinSet(GPIOC, 15);

                // Wait 0 at PC13
                while(PinIsSet(GPIOC, 13)) chThdSleepMilliseconds(99);

                Led.SetColor(clBlack);
                // Set 0 at PC15
                PinClear(GPIOC, 15);
            }
        }
        else {
            Led.SetColor(clBlack);
        }
        chThdSleepMilliseconds(99);
#endif
}
#endif // task

#if 1 // ============================
void rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
#endif
    // Init radioIC
    CC.Init();
    CC.SetTxPower(CC_Pwr0dBm);
    CC.SetPktSize(RPKT_LEN);
    CC.SetChannel(9);
    // Thread
    chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
