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
#include "indication.h"

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

//#define TX
//#define LED_RX
void rLevel1_t::ITask() {
#if 1 // ======== TX cycle ========
    switch(App.Type) {
        case dtLustraClean:
        case dtLustraWeak:
        case dtLustraStrong:
        case dtLustraLethal:
            // Setup channel, do nothing if bad ID
            if((App.ID >= LUSTRA_MIN_ID) and (App.ID <= LUSTRA_MAX_ID)) {
                CC.SetChannel(LUSTRA_ID_TO_RCHNL(App.ID));
                // Transmit corresponding pkt
                uint8_t Indx = App.Type - dtLustraClean;
                CC.TransmitSync((void*)&PktLustra[Indx]);
            }
            else {
                Indication.LustraBadID();
                chThdSleepMilliseconds(999);
                return;
            }
            break;

        case dtPelengator:
            CC.SetChannel(RCHNL_PELENG);
            for(uint8_t i=0; i<PELENG_TX_CNT; i++) CC.TransmitSync((void*)&PktPelengator);
            break;

        default: break;
    } // switch
#endif

#if 1 // ======== RX cycle ========
    int8_t Rssi;
    uint8_t RxRslt = FAILURE;
    uint32_t TimeElapsed;
    // Everyone listen to pelengator
    CC.SetChannel(RCHNL_PELENG);
    RxRslt = CC.ReceiveSync(PELENG_RX_T_MS, &PktRx, &Rssi);
    if(RxRslt == OK) {
        int32_t RssiPercent = dBm2Percent(Rssi);
//        Uart.Printf("Peleng %d\r", RssiPercent);
        if(RssiPercent > RLVL_PELENGATOR) {
            chSysLock();
            chEvtSignalI(Indication.PThd, EVTMSK_PELENG_FOUND);
            chSysUnlock();
            return; // Get out if pelengator found
        }
    } // if OK
    // If pelengator not found, listen other channels
    switch(App.Type) {
        case dtNothing: chThdSleepMilliseconds(999); break;

        case dtUmvos:
        case dtDetectorMobile:
        case dtDetectorFixed:
            // Supercycle
            for(uint32_t j=0; j<CYCLE_CNT; j++) {
                // Iterate channels
                for(uint8_t i=RCHNL_MIN; i<RCHNL_MAX; i++) {
                    CC.SetChannel(i);
                    RxRslt = CC.ReceiveSync(LUSTRA_RX_T_MS, &PktRx, &Rssi);
                    if(RxRslt == OK) {
//                            Uart.Printf("Ch=%u; Lvl=%d\r", i, Rssi);
                        App.RxTable.PutPkt(i, &PktRx, Rssi);
                    }
                } // for i
            } // for j
            // Supercycle completed, switch table
            TimeElapsed = App.RxTable.PWriteTbl->Age();
            if(TimeElapsed < 1000) chThdSleepMilliseconds(1000 - TimeElapsed);
            // ...and inform application
            chSysLock();
            App.RxTable.SwitchTableI();
            chEvtSignalI(App.PThd, EVTMSK_RX_TABLE_READY);
            chSysUnlock();
            break;

        default: break;
    } // switch
#endif // RX

#ifdef TX
        // Transmit
        DBG1_SET();
        CC.TransmitSync(&PktTx);
        DBG1_CLR();
        //chThdSleepMilliseconds(99);
#elif defined LED_RX
        Color_t Clr;
        int8_t Rssi;
//        if(Enabled) {
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
            else {
                Clr = clBlack;
    //            Uart.Printf("Halt\r");
            }
            Led.SetColor(Clr);
//        }
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
    // Thread
    chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
