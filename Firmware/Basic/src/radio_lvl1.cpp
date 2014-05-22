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

#include "mesh_lvl.h"

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
    while(true) {
        if(App.Type == dtUmvos) {
            if(Mesh.IsInit) {
                uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS); /* wait mesh cycle */

                CC.SetChannel(MESH_CHANNEL); /* set mesh channel */
                CC.SetPktSize(MESH_PKT_SZ);
                if(EvtMsk & EVTMSK_MESH_RX) IMeshRx();

                if(EvtMsk & EVTMSK_MESH_TX) {
                    CC.TransmitSync(&Mesh.PktTx); /* Pkt was prepared in Mesh Thd */

//                Uart.Printf("rTxPkt: %u %u %u %u  {%u %u %u %d %u %u %u}\r",
//                        Mesh.PktTx.MeshData.SelfID,
//                        Mesh.PktTx.MeshData.CycleN,
//                        Mesh.PktTx.MeshData.TimeOwnerID,
//                        Mesh.PktTx.MeshData.TimeAge,
//                        Mesh.PktTx.PayloadID,
//                        Mesh.PktTx.Payload.Hops,
//                        Mesh.PktTx.Payload.Timestamp,
//                        Mesh.PktTx.Payload.TimeDiff,
//                        Mesh.PktTx.Payload.Reason,
//                        Mesh.PktTx.Payload.Location,
//                        Mesh.PktTx.Payload.Emotion
//                        );
//                    IIterateChannels(); /* Mesh pkt was transmited now lets check channels */
                } // Mesh Tx
            } // Mesh Init
            else chThdSleepMilliseconds(999);
        }
        else chThdSleepMilliseconds(999);
    }
}
#endif


#ifdef DEVTYPE_UMVOS
        if(Mesh.IsInit) {
            uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS); /* wait mesh cycle */

            CC.SetChannel(MESH_CHANNEL); /* set mesh channel */
            CC.SetPktSize(MESH_PKT_SZ);
            if(EvtMsk & EVTMSK_MESH_RX) IMeshRx();

            if(EvtMsk & EVTMSK_MESH_TX) {
                CC.TransmitSync(&Mesh.PktTx); /* Pkt was prepared in Mesh Thd */
//                Uart.Printf("rTxPkt: %u %u %u %u  {%u %u %u %d %u %u %u}\r",
//                        Mesh.PktTx.MeshData.SelfID,
//                        Mesh.PktTx.MeshData.CycleN,
//                        Mesh.PktTx.MeshData.TimeOwnerID,
//                        Mesh.PktTx.MeshData.TimeAge,
//                        Mesh.PktTx.PayloadID,
//                        Mesh.PktTx.Payload.Hops,
//                        Mesh.PktTx.Payload.Timestamp,
//                        Mesh.PktTx.Payload.TimeDiff,
//                        Mesh.PktTx.Payload.Reason,
//                        Mesh.PktTx.Payload.Location,
//                        Mesh.PktTx.Payload.Emotion
//                        );
                IIterateChannels(); /* Mesh pkt was transmited now lets check channels */
            }
        }
        else IIterateChannels(); /* Mesh not Init */

#if defined DEVTYPE_UMVOS || defined DEVTYPE_DETECTOR
        int8_t Rssi;
        // Supercycle
        for(uint32_t j=0; j<CYCLE_CNT; j++) {
            // Iterate channels
            for(uint8_t i=RCHNL_MIN; i<RCHNL_MAX; i++) {
                CC.SetChannel(i);
                uint8_t RxRslt = CC.ReceiveSync(RX_T_MS, &PktRx, &Rssi);
                if(RxRslt == OK) {
//                    Uart.Printf("Ch=%u; T=%u; Lvl=%d\r", i, PktRx.Type, Rssi);
                    App.RxTable.PutPkt(i, &PktRx, Rssi);
                }
            } // for i
        } // for j

        // Supercycle completed, switch table and inform application
        uint32_t TimeElapsed = chTimeNow() - LastTime;
        if(TimeElapsed < 1000) chThdSleepMilliseconds(1000 - TimeElapsed);
        LastTime = chTimeNow();

        chSysLock();
        App.RxTable.SwitchTableI();
        chEvtSignalI(App.PThd, EVTMSK_RX_TABLE_READY);
        chSysUnlock();
>>>>>>> ConsoleMesh
//        Uart.Printf("***\r");
#elif defined DEVTYPE_LUSTRA
        DBG1_SET();
        CC.TransmitSync(&PktTx);
        DBG1_CLR();
#endif

#ifdef SHIKO_DEVICE // ======== TX cycle ========
        PktTx.Type = App.Type;
        switch(App.Type) {
            case dtFieldWeak:
            case dtFieldNature:
            case dtFieldStrong:
                CC.SetChannel(App.ID);
                CC.TransmitSync(&PktTx);
                break;
=======
#if 1 // ======== TX cycle ========
    uint8_t Indx;
    switch(App.Type) {
        case dtLustraClean:
        case dtLustraWeak:
        case dtLustraStrong:
        case dtLustraLethal:
            // Setup channel, do nothing if bad ID
            if((App.ID < LUSTRA_MIN_ID) or (App.ID > LUSTRA_MAX_ID)) {
                Led.StartBlink(LedBadID);
                chThdSleepMilliseconds(999);
                return;
            }
            CC.SetChannel(LUSTRA_ID_TO_RCHNL(App.ID));
            // Transmit corresponding pkt
            Indx = App.Type - dtLustraClean;
            CC.TransmitSync((void*)&PktLustra[Indx]);
            break;
>>>>>>> origin/Fallout

//        case dtDetector:
//            CC.SetChannel(FIELD_RX_CHNL);
//            for(uint8_t i=0; i<DETECTOR_TX_CNT; i++) CC.TransmitSync(&PktTx);
//            break;
        default: break;
    } // switch
#endif

#if 1 // ======== RX cycle ========
    int8_t Rssi;
    switch(App.Type) {
        case dtUmvos:
            // Supercycle
            for(uint32_t j=0; j<CYCLE_CNT; j++) {
                // Iterate channels
                for(uint8_t i=RCHNL_MIN; i<RCHNL_MAX; i++) {
                    CC.SetChannel(i);
                    uint8_t RxRslt = CC.ReceiveSync(RX_T_MS, &PktRx, &Rssi);
                    if(RxRslt == OK) {
//                        Uart.Printf("Ch=%u; T=%u; Lvl=%d\r", i, PktRx.Type, Rssi);
                        App.RxTable.PutPkt(i, &PktRx, Rssi);
                    }
                } // for i
            } // for j
            // Supercycle completed, switch table
            uint32_t TimeElapsed = chTimeNow() - LastTime;
            if(TimeElapsed < 1000) chThdSleepMilliseconds(1000 - TimeElapsed);
            LastTime = chTimeNow();
            // ...and inform application
            chSysLock();
            App.RxTable.SwitchTableI();
            chEvtSignalI(App.PThd, EVTMSK_RX_TABLE_READY);
            chSysUnlock();
            break;

        default:
//            chThdSleepMilliseconds(999);
            break;
    } // switch
        // Everyone
//        CC.SetChannel(FIELD_RX_CHNL);
//        RxRslt = CC.ReceiveSync(FIELD_RX_T_MS, &PktRx, &Rssi);
//        if((RxRslt == OK) and (PktRx.Type == (uint8_t)dtDetector)) {
//            Uart.Printf("Ch=%u; T=%u; Lvl=%d\r", FIELD_RX_CHNL, PktRx.Type, Rssi);
//            int32_t RssiPercent = dBm2Percent(Rssi);
//            App.DetectorFound(RssiPercent);
//        }
//        // If detector not found, listen other channels
//        else {
//            if(ANY_OF_4(App.Type, dtXtraNormal, dtXtraWeak, dtUfo, dtDetector)) {
//                for(uint8_t i=RCHNL_MIN; i<RCHNL_MAX; i++) {
//                    CC.SetChannel(i);
//                    RxRslt = CC.ReceiveSync(RCVR_RX_T_MS, &PktRx, &Rssi);
//                    if(RxRslt == OK) {
////                        Uart.Printf("Ch=%u; T=%u; Lvl=%d\r", i, PktRx.Type, Rssi);
//                        App.RxTable.PutInfo(i, PktRx.Type, Rssi);
//                    }
//                } // for
//            } // if any of
//        } // if detector found
//        Uart.Printf("***\r");
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
#endif
//        if(App.Type == dtNothing) chThdSleepMilliseconds(999);


#if 1 // ==== Iterate Channels ====
//void rLevel1_t::IIterateChannels() {
//    int8_t Rssi;
//    /* Iterate Lustrs */
//    CC.SetPktSize(RPKT_LEN);
//    for(uint32_t j=0; j<CYCLE_CNT; j++) {
//        // Iterate channels
//        for(uint8_t i=RCHNL_MIN; i<RCHNL_MAX; i++) {
//            CC.SetChannel(i);
//            uint8_t RxRslt = CC.ReceiveSync(RX_T_MS, &PktRx, &Rssi);
//            if(RxRslt == OK) {
////                    Uart.Printf("Ch=%u; T=%u; Lvl=%d\r", i, PktRx.Type, Rssi);
////                App.RxTable.PutPkt(&PktRx, Rssi);
//            }
//        } // for i
//    } // for j
//    /* Iterate Lustrs completed, switch table and inform application */
//
//    chSysLock();
////    App.RxTable.SwitchTableI();
//    chEvtSignalI(App.PThd, EVTMSK_RX_TABLE_READY);
//    chSysUnlock();
//}
#endif


#if 1 // ==== Mesh Rx Cycle ====

static void RxEnd(void *p) {
//    Uart.Printf("RxTmt, t=%u\r", chTimeNow());
    Radio.Valets.InRx = false;
}

void rLevel1_t::IMeshRx() {
    int8_t RSSI = 0;
    Valets.RxTmt = CYCLE_TIME;
    Valets.InRx = true;
    chVTSet(&Valets.RxVT, MS2ST(CYCLE_TIME), RxEnd, nullptr); /* Set VT */
    do {
        Valets.CurrentTime = chTimeNow();
        uint8_t RxRslt = CC.ReceiveSync(Valets.RxTmt, &Mesh.PktRx, &RSSI);
        if(RxRslt == OK) { // Pkt received correctly
            Uart.Printf("ID=%u:%u, %ddBm\r", Mesh.PktRx.MeshData.SelfID, Mesh.PktRx.MeshData.CycleN, RSSI);
            Payload.WriteInfo(Mesh.PktRx.MeshData.SelfID, RSSI, Mesh.GetCycleN(), &Mesh.PktRx.Payload);
            Mesh.MsgBox.Post({chTimeNow(), Mesh.PktRx.MeshData}); /* SendMsg to MeshThd with PktRx structure */
        } // Pkt Ok
        Valets.RxTmt = ((chTimeNow() - Valets.CurrentTime) > 0)? Valets.RxTmt - (chTimeNow() - Valets.CurrentTime) : 0;
    } while(Radio.Valets.InRx);
    Mesh.SendEvent(EVTMSK_MESH_UPD_CYC);
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
    CC.SetPktSize(RPKT_LEN);
    // Thread
    rThd = chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
