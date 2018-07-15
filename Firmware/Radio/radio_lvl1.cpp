/*
 * radio_lvl1.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#include "radio_lvl1.h"
#include "cc1101.h"
#include "MsgQ.h"
#include "led.h"
#include "Sequences.h"
#include "EvtMsgIDs.h"
#include "main.h"

cc1101_t CC(CC_Setup0);

#define DBG_PINS

#ifdef DBG_PINS
#define DBG_GPIO1   GPIOC
#define DBG_PIN1    13
#define DBG1_SET()  PinSetHi(DBG_GPIO1, DBG_PIN1)
#define DBG1_CLR()  PinSetLo(DBG_GPIO1, DBG_PIN1)
//#define DBG_GPIO2   GPIOB
//#define DBG_PIN2    9
//#define DBG2_SET()  PinSet(DBG_GPIO2, DBG_PIN2)
//#define DBG2_CLR()  PinClear(DBG_GPIO2, DBG_PIN2)
#else
#define DBG1_SET()
#define DBG1_CLR()
#endif

rLevel1_t Radio;
extern Timer_t SyncTmr;
extern Mode_t Mode;

#if 1 // ================================ Task =================================
static THD_WORKING_AREA(warLvl1Thread, 256);
__noreturn
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    Radio.ITask();
}

void rLevel1_t::TryToSleep(uint32_t SleepDuration) {
    if(SleepDuration >= MIN_SLEEP_DURATION_MS) CC.EnterPwrDown();
    chThdSleepMilliseconds(SleepDuration);
}

__noreturn
void rLevel1_t::ITask() {
    while(true) {
        CC.Recalibrate();
        uint8_t RxRslt = CC.Receive(180, &PktRx, RPKT_LEN, &Rssi);
        if(RxRslt == retvOk) {
//            Printf("Rssi=%d\r", Rssi);
//            PktRx.Print();
            if(Mode == modeSync) SyncTmr.SetCounter(PktRx.Time); // Sync timer
            EvtMsg_t Msg;
            Msg.ID = evtIdNewRadioCmd;
            Msg.w16[0] = PktRx.Mode;
            Msg.w16[1] = PktRx.ColorH;
            Msg.w16[2] = PktRx.Period;
            EvtQMain.SendNowOrExit(Msg);
        }
        TryToSleep(360);
    } // while
}
#endif // task

#if 1 // ============================
uint8_t rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
//    PinSetupOut(DBG_GPIO2, DBG_PIN2, omPushPull);
#endif    // Init radioIC

    if(CC.Init() == retvOk) {
        CC.SetPktSize(RPKT_LEN);
        CC.SetChannel(0);
        CC.SetTxPower(CC_TX_PWR);

        // Thread
        chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
        return retvOk;
    }
    else return retvFail;
}
#endif
