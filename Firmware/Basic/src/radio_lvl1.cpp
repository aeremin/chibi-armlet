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
__attribute__((noreturn))
void rLevel1_t::ITask() {
    int8_t Rssi;
    uint8_t RxRslt;
    while(true) {
        PktTx.Type = App.Type;

#if 1 // ======== TX cycle ========
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

#if 1 // ======== RX cycle ========
        switch(App.Type) {
            case dtFieldWeak:
            case dtFieldNature:
            case dtFieldStrong:
                CC.SetChannel(FIELD_RX_CHNL);
                RxRslt = CC.ReceiveSync(FIELD_RX_T_MS, &PktRx, &Rssi);
                if(RxRslt == OK) {
                    Uart.Printf("Ch=%u; T=%u; Lvl=%d\r", FIELD_RX_CHNL, PktRx.Type, Rssi);
                    if(PktRx.Type == (uint8_t)dtDetector) Led.StartBlink(LedFieldDemonstrate);
                }
                break;

            case dtXtraNormal:
            case dtXtraWeak:
            case dtUfo:
            case dtDetector:
                for(uint8_t i=RCHNL_MIN; i<RCHNL_MAX; i++) {
                    CC.SetChannel(i);
                    RxRslt = CC.ReceiveSync(RCVR_RX_T_MS, &PktRx, &Rssi);
                    if(RxRslt == OK) {
//                        Uart.Printf("Ch=%u; T=%u; Lvl=%d\r", i, PktRx.Type, Rssi);
                        App.RxTable.PutInfo(i, PktRx.Type, Rssi);
                    }
                } // for
                App.RxTable.SendEvtReady();
                break;

            default:
                chThdSleepMilliseconds(450);
                break;
        } // switch
#endif

#ifdef TX
        // Transmit
        DBG1_SET();
        CC.TransmitSync(&PktTx, RPKT_LEN);
        DBG1_CLR();
        chThdSleepMilliseconds(99);
#elif defined LED_RX
        Color_t Clr;
        int8_t Rssi;
        RxRslt = CC.ReceiveSync(306, &PktRx, RPKT_LEN, &Rssi);
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
    PktTx.Check[0] = CHECK_0;
    PktTx.Check[1] = CHECK_1;
    PktTx.Check[2] = CHECK_2;
    // Thread
    PThread = chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
