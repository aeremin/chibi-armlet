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
#include "sequences.h"

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
__attribute__((noreturn))
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    while(true) Radio.ITask();
}

//#define TX
#define LED_RX
__attribute__((noreturn))
void rLevel1_t::ITask() {
    uint8_t RxRslt;
    while(true) {
#ifdef SHIKO_DEVICE // ======== TX cycle ========
        PktTx.Type = App.Type;
        switch(App.Type) {
            case dtFieldWeak:
            case dtFieldNature:
            case dtFieldStrong:
                CC.SetChannel(App.ID);
                CC.TransmitSync(&PktTx);
                break;

            case dtDetector:
                CC.SetChannel(FIELD_RX_CHNL);
                for(uint8_t i=0; i<DETECTOR_TX_CNT; i++) CC.TransmitSync(&PktTx);
                break;
            default: break;
        } // switch
#endif

#ifdef SHIKO_DEVICE // ======== RX cycle ========
        // Everyone
        CC.SetChannel(FIELD_RX_CHNL);
        RxRslt = CC.ReceiveSync(FIELD_RX_T_MS, &PktRx, &Rssi);
        if((RxRslt == OK) and (PktRx.Type == (uint8_t)dtDetector)) {
//            Uart.Printf("Ch=%u; T=%u; Lvl=%d\r", FIELD_RX_CHNL, PktRx.Type, Rssi);
            int32_t RssiPercent = dBm2Percent(Rssi);
            App.DetectorFound(RssiPercent);
        }
        // If detector not found, listen other channels
        else {
            if(ANY_OF_4(App.Type, dtXtraNormal, dtXtraWeak, dtUfo, dtDetector)) {
                for(uint8_t i=RCHNL_MIN; i<RCHNL_MAX; i++) {
                    CC.SetChannel(i);
                    RxRslt = CC.ReceiveSync(RCVR_RX_T_MS, &PktRx, &Rssi);
                    if(RxRslt == OK) {
//                        Uart.Printf("Ch=%u; T=%u; Lvl=%d\r", i, PktRx.Type, Rssi);
                        App.RxTable.PutInfo(i, PktRx.Type, Rssi);
                    }
                } // for
            } // if any of
        } // if detector found
//        Uart.Printf("***\r");
#endif

#ifdef TX
        // Transmit
        DBG1_SET();
        CC.TransmitSync(&PktTx);
        DBG1_CLR();
        //chThdSleepMilliseconds(99);
#elif defined LED_RX
        Color_t Clr;
        int8_t Rssi;
        RxRslt = CC.ReceiveSync(306, &PktRx, &Rssi);
        if(RxRslt == OK) {
            Uart.Printf("%d\r", Rssi);
            Clr = clWhite;
            if     (Rssi < -100) Clr = clRed;
            else if(Rssi < -90) Clr = clYellow;
            else if(Rssi < -80) Clr = clGreen;
            else if(Rssi < -70) Clr = clCyan;
            else if(Rssi < -60) Clr = clBlue;
            else if(Rssi < -50) Clr = clMagenta;
        }
        else Clr = clBlack;
        Led.SetColor(Clr);
        chThdSleepMilliseconds(99);
#endif
    } // while true
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
    CC.SetChannel(0);
    CC.SetPktSize(RPKT_LEN);
    // Variables

    // Thread
    chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
