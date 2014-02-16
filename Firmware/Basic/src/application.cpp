/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
#include "cmd_uart.h"
#include "pill_mgr.h"
#include "peripheral.h"
#include "sequences.h"
#include "evt_mask.h"
#include "eestore.h"
#include "radio_lvl1.h"
#include "AccGiro.h"

App_t App;
#define UART_RPL_BUF_SZ     36
//static uint8_t SBuf[UART_RPL_BUF_SZ];

#if 1 // ============================ Timers ===================================
static VirtualTimer ITmrCheck;
void TmrCheckCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_CHECK);
    chVTSetI(&ITmrCheck, MS2ST(TM_CHECK_MS), TmrCheckCallback, nullptr);
    chSysUnlockFromIsr();
}
#endif

#if 1 // ========================= Application =================================
void App_t::ITask() {
    uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
    if(EvtMsk & EVTMSK_CHECK) {
        AccGyro.ReadAccelerations();
        Uart.Printf("Acc: %d %d %d\r", AccGyro.a[0], AccGyro.a[1], AccGyro.a[2]);

    } // if EVTMSK_PILL_CHECK
}

void App_t::Init() {
    PThd = chThdSelf();
    // Timers init
    chSysLock();
    chVTSetI(&ITmrCheck, MS2ST(TM_CHECK_MS), TmrCheckCallback, nullptr);
    chSysUnlock();
}
#endif

#if 1 // ======================= Command processing ============================
void Ack(uint8_t Result) { Uart.Cmd(0x90, &Result, 1); }

void UartCmdCallback(uint8_t CmdCode, uint8_t *PData, uint32_t Length) {
    switch(CmdCode) {
        case CMD_PING: Ack(OK); break;

        default: break;
    } // switch
}
#endif
