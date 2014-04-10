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

#define RECALIBRATE_TIME   5000 /* in ms */

#ifdef  MESH
static void RxEnd(void *p) {
    Uart.Printf("RxTmt, t=%u\r", chTimeNow());
    Radio.IMeshRx = false;
}
#endif

void rLevel1_t::ITask() {
    while(true) {
        // New cycle begins
        CC.Recalibrate();   // Recalibrate manually every cycle, as auto recalibration disabled
        int8_t RSSI = 0;
        chVTSet(&MeshRxVT, MS2ST(RECALIBRATE_TIME), RxEnd, nullptr); /* Set VT */
        Uart.Printf("Recalibrate\r");
        IMeshRx = true;
        do {
            uint8_t RxRslt = CC.ReceiveSync(CYCLE_TIME, &PktRx, &RSSI);
            if(RxRslt == OK) { // Pkt received correctly
                Uart.Printf("ID=%u, %d, %u\r", PktRx.ID, RSSI, chTimeNow());
            } // Pkt Ok
        } while(IMeshRx);
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

    PktTx.ID = (uint8_t)ASelfID;
    PktTx.CycleN = 0;
    PktTx.TimeOwnerID = PktTx.ID;
    ResetTimeAge(PktTx.ID);
#endif


    // Init radioIC
    CC.Init();
    CC.SetTxPower(CC_Pwr0dBm);
    CC.SetChannel(MESH_CHANNEL);
    // Variables
    Damage = 1;
    // Thread
    PrThd = PThread = chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
