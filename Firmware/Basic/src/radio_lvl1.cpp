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

#ifdef  MESH
static void RxEnd(void *p) {
//    Uart.Printf("RxTmt, t=%u\r", chTimeNow());
    Radio.IMeshRx = false;
}
#endif

void rLevel1_t::ITask() {
    while(true) {
        // New cycle begins
        CC.Recalibrate();   // Recalibrate manually every cycle, as auto recalibration disabled

#ifdef TX
        // Transmit
        DBG1_SET();
        CC.TransmitSync(&PktTx);
        DBG1_CLR();
        chThdSleepMilliseconds(99);
#elif defined LED_RX
        Color_t Clr;
        uint8_t RxRslt = CC.ReceiveSync(306, &PktRx);
        if(RxRslt == OK) {
            Uart.Printf("%d\r", PktRx.RSSI);
            Clr = clWhite;
            if     (PktRx.RSSI < -100) Clr = clRed;
            else if(PktRx.RSSI < -90) Clr = clYellow;
            else if(PktRx.RSSI < -80) Clr = clGreen;
            else if(PktRx.RSSI < -70) Clr = clCyan;
            else if(PktRx.RSSI < -60) Clr = clBlue;
            else if(PktRx.RSSI < -50) Clr = clMagenta;
        }
        else Clr = clBlack;
        Led.SetColor(Clr);
#else
//        IterateEmanators();
//        Uart.Printf("%d\r", Damage);
//        chThdSleepMilliseconds(45);
#endif
#if 1 // ==== Mesh ====
        CC.SetChannel(MESH_CHANNEL);
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
        PktTx.TimeAge++;
        if(EvtMsk & EVTMSK_MESH_TX) {
            PktTx.CycleN = Mesh.GetCycleN();
//            Uart.Printf("RadioTx\r");
            if(PktTx.TimeAge > TIME_AGE_THRESHOLD) { ResetTimeAge(PktTx.SelfID); }
            Led.StartBlink(LedBlueOneBlink);
            CC.TransmitSync(&PktTx);
        }
        if(EvtMsk & EVTMSK_MESH_RX) {
            int8_t RSSI = 0;
            RxTmt = CYCLE_TIME;
            IMeshRx = true;
            chVTSet(&MeshRxVT, MS2ST(CYCLE_TIME), RxEnd, nullptr); /* Set VT */
            Led.StartBlink(LedRadioRx);
            do {
                Time = chTimeNow();
                uint8_t RxRslt = CC.ReceiveSync(RxTmt, &PktRx, &RSSI);
                if(RxRslt == OK) { // Pkt received correctly
                    Uart.Printf("ID=%u:%u, %ddBm\r", PktRx.SelfID, PktRx.CycleN, RSSI);
                    Payload.WriteInfo(PktRx.SelfID, RSSI, &PktRx.PktPayload);
                    Mesh.MsgBox.Post({chTimeNow(), PktRx.MeshPayload}); /* SendMsg to MeshThd with PktRx structure */
                } // Pkt Ok
                RxTmt = ((chTimeNow() - Time) > 0)? RxTmt - (chTimeNow() - Time) : 0;
            } while(IMeshRx);
            Mesh.SendEvent(EVTMSK_UPDATE_CYCLE);
            Led.StopBlink();
        }
#endif
    } // while true
}

#endif

#if 1 // ============================
void rLevel1_t::Init(uint16_t ASelfID) {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
#endif

#ifdef MESH
    if(ASelfID == 0) {
        Uart.Printf("rLvl1 WrongID\r");
        return;
    }
    // Prepare Debug Pkt
    PktTx.SelfID = (uint8_t)ASelfID;
    PktTx.CycleN = 0;
    PktTx.TimeOwnerID = PktTx.SelfID;
    ResetTimeAge(PktTx.SelfID);
    // PayLoad
    PktTx.ID = 45;
    PktTx.PktPayload.Hops = 6;
    PktTx.PktPayload.Timestamp = 65432;
    PktTx.PktPayload.TimeDiff = 9870;
    PktTx.PktPayload.Location = 1;
    PktTx.PktPayload.Reason = 5;
    PktTx.PktPayload.Emotion = 8;
#endif


    // Init radioIC
    CC.Init();
    CC.SetTxPower(CC_Pwr0dBm);
    CC.SetChannel(CHANNEL_ZERO);
    // Variables
    Damage = 1;
    // Thread
    PrThd = PThread = chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
