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
        NewDamageReady = false;
        Damage = 1;
        // Iterate slow emanators
        for(uint8_t i=0; i<SLOW_EMANATOR_CNT; i++) {
            CC.SetChannel(CHANNEL_ZERO + i);
            uint8_t RxRslt = CC.ReceiveSync(27, &PktRx);
            if(RxRslt == OK) {
//                Uart.Printf("%d\r", PktRx.ConstDmg);
                // "Clean zone" emanator
                if((PktRx.ConstDmg == 0) and (PktRx.VarDmgMax == 0) and (PktRx.VarDmgMin == 0)) {
                    if(Damage > 0) Damage--;
                }
                else {
                    Damage += PktRx.ConstDmg;
                }

                if(Damage < PktRx.ConstDmg) Damage = PktRx.ConstDmg;
                NewDamageReady = true;  // Set to true if any pkt is received
            } // if ok
        } // for
        // Sleep until asked
        CC.Sleep();
        if(NewDamageReady) {    // Wait until read
            chSysLock();
            chSchGoSleepS(THD_STATE_SUSPENDED);
            chSysUnlock();
        }
        else chThdSleepMilliseconds(27);
#endif
    } // while true
}

uint32_t rLevel1_t::GetDamage() {
    if(!NewDamageReady) return 1;
    uint32_t FDmg = Damage;
    chSysLock();
    if(PThread->p_state == THD_STATE_SUSPENDED) {
        PThread->p_u.rdymsg = RDY_OK;    // Signal that IRQ fired
        chSchReadyI(PThread);
    }
    chSysUnlock();
    return FDmg;    // Otherwise Damage can be changed right after high-prio thread wakeup
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
    NewDamageReady = false;
    // Thread
    PThread = chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
