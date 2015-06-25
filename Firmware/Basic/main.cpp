/*
 * File:   main.cpp
 * Author: Kreyl
 * Project: Armlet2South
 *
 * Created on Feb 05, 2013, 20:27
 */

#include <main.h>
#include "kl_lib.h"
#include "ch.h"
#include "hal.h"
#include "clocking_L1xx.h"
#include "uart.h"
#include "radio_lvl1.h"
#include "evt_mask.h"
#include "vibro.h"
#include "led.h"
#include "ws2812b.h"

LedRGB_t Led({GPIOB, 1, TIM3, 4}, {GPIOB, 0, TIM3, 3}, {GPIOB, 5, TIM3, 2});
LedWs_t LedWs;

App_t App;

#if 1 // ============================ Timers ===================================
// Universal VirtualTimer callback
void TmrGeneralCallback(void *p) {
    chSysLockFromIsr();
    App.SignalEvtI((eventmask_t)p);
    chSysUnlockFromIsr();
}
// Check radio buffer
void TmrCheckCallback(void *p) {
    chSysLockFromIsr();
    App.SignalEvtI(EVTMSK_CHECK);
    chVTSetI(&App.TmrCheck, MS2ST(RX_CHECK_PERIOD_MS), TmrCheckCallback, nullptr);
    chSysUnlockFromIsr();
}
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
    Clk.SetMSIRange(msr4MHz);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();

    // ==== Init Hard & Soft ====
    App.InitThread();
    Uart.Init(115200);
    Uart.Printf("\r%S_%S; AHB=%u", APP_NAME, APP_VERSION, Clk.AHBFreqHz);

    Led.Init();
    LedWs.Init();
    LedWs.StartSequence(wsqPwrOn);

    if(Radio.Init() != OK) Led.StartSequence(lsqFailure);
    else Led.StartSequence(lsqStart);

    // Timers
    chVTSet(&App.TmrCheck, MS2ST(RX_CHECK_PERIOD_MS), TmrCheckCallback, nullptr);

    // Main cycle
    App.ITask();
}


__attribute__ ((__noreturn__))
void App_t::ITask() {
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
#if 1   // ==== Uart cmd ====
        if(EvtMsk & EVTMSK_UART_NEW_CMD) {
            OnUartCmd(&Uart);
            Uart.SignalCmdProcessed();
        }
#endif

#if 1   // ==== Rx buf check ====
        if(EvtMsk & EVTMSK_CHECK) {
            // Get number of distinct received IDs and clear table
            chSysLock();
            uint32_t Cnt = Radio.RxTable.GetCount();
            Radio.RxTable.Clear();
            chSysUnlock();
//            if(Cnt != 0)
//                Uart.Printf("\rCnt = %u  %u", Cnt, chTimeNow());
            // ==== Select indication depending on Cnt ====
#if 0   // Inner LED
            const LedRGBChunk_t *lsq = lsqNone;
            if(Cnt == 1 or Cnt == 2) lsq = lsqOneOrTwo;
            else if(Cnt > 2) lsq = lsqMany;

            if(lsq != lsqSaved) {
                lsqSaved = lsq;
                Led.StartSequence(lsq);
            }
#endif
            // Restart TmrOff if someone is near
            if(Cnt != 0) chVTRestart(&TmrOff, INDICATION_TIME_MS, EVTMSK_OFF);
            // Setup indication
            if((Cnt == 1 or Cnt == 2) and (wsqSaved == wsqNone)) {
                Uart.Printf("\rOneTwo %u", chTimeNow());
                wsqSaved = wsqOneOrTwo;
                LedWs.StartSequence(wsqSaved);
            }
            else if((Cnt > 2) and (wsqSaved == wsqNone or wsqSaved == wsqOneOrTwo)) {
                Uart.Printf("\rMany %u", chTimeNow());
                wsqSaved = wsqMany;
                LedWs.StartSequence(wsqSaved);
            }
        }
#endif

#if 1 // ==== Off ====
        if(EvtMsk & EVTMSK_OFF) {
            Uart.Printf("\rOff %u", chTimeNow());
            wsqSaved = wsqNone;
            LedWs.StartSequence(wsqSaved);
        }
#endif
    } // while true
}

void App_t::OnUartCmd(Uart_t *PUart) {
    UartCmd_t *PCmd = &PUart->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
    Uart.Printf("\r%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) PUart->Ack(OK);

//    else if(PCmd->NameIs("GetID")) Uart.Reply("ID", ID);

//    else if(PCmd->NameIs("SetID")) {
//        if(PCmd->GetNextToken() != OK) { PUart->Ack(CMD_ERROR); return; }
//        if(PCmd->TryConvertTokenToNumber(&dw32) != OK) { PUart->Ack(CMD_ERROR); return; }
//        uint8_t r = ISetID(dw32);
//        PUart->Ack(r);
//    }

    else PUart->Ack(CMD_UNKNOWN);
}
