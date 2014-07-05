/*
 * File:   main.cpp
 * Author: Kreyl
 * Project: Armlet2South
 *
 * Created on Feb 05, 2013, 20:27
 */

#include "kl_lib_L15x.h"
#include "ch.h"
#include "hal.h"
#include "clocking_L1xx.h"
#include "pill_mgr.h"
#include "cmd_uart.h"
#include "evt_mask.h"
#include "cc1101defins.h"
#include "rlvl1_defins.h"
#include "cc1101.h"

// Device type
#define TRANSMITTER
#ifndef TRANSMITTER
#define RECEIVER
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V2);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();
    // ==== Init Hard & Soft ====
    Uart.Init(115200);
    Uart.Printf("\rRst\r\n");
    PillMgr.Init();

    // Init radioIC
    CC.Init();
    CC.SetTxPower(CC_Pwr0dBm);
    CC.SetPktSize(RPKT_LEN);
    CC.SetChannel(4);

    rPkt_t rPkt;

    while(true) {
#ifdef TRANSMITTER
        chThdSleepMilliseconds(207);
        rPkt.AccX = 1;
        rPkt.AccY = 2;
        rPkt.AccZ = 3;
        CC.TransmitSync((void*)&rPkt);
#else
        int8_t Rssi;
        uint8_t RxRslt = CC.ReceiveSync(360, &rPkt, &Rssi);
        if(RxRslt == OK) {
            Uart.Printf(
                    "%u %u %u %u %u %u %u %u %u\r\n",
                    rPkt.AccX, rPkt.AccY, rPkt.AccZ,
                    rPkt.AngleU, rPkt.AngleV, rPkt.AngleW,
                    rPkt.AngVelU, rPkt.AngVelV, rPkt.AngVelW
            );
        }
#endif
    } // while true
}
