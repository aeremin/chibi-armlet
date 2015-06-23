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
#include "Sequences.h"

LedRGB_t Led({GPIOB, 1, TIM3, 4}, {GPIOB, 0, TIM3, 3}, {GPIOB, 5, TIM3, 2});

App_t App;

#if 1 // ============================ Timers ===================================
void TmrCheckCallback(void *p) {
    chSysLockFromIsr();
    App.SignalEvtI(EVTMSK_RX_BUF_CHECK);
    chVTSetI(&App.TmrCheck, MS2ST(RXBUF_CHECK_PERIOD_MS), TmrCheckCallback, nullptr);
    chSysUnlockFromIsr();
}
void TmrIndicationCallback(void *p) {
    chSysLockFromIsr();
    App.SignalEvtI(EVTMSK_INDICATION);
    chVTSetI(&App.TmrIndication, MS2ST(INDICATION_PERIOD_MS), TmrIndicationCallback, nullptr);
    chSysUnlockFromIsr();
}
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V2);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();

    // ==== Init Hard & Soft ====
    App.InitThread();
    Uart.Init(115200);
    GetMcuUID(&App.UID, nullptr, nullptr);
    Uart.Printf("\r%S_%S", APP_NAME, APP_VERSION);

    Led.Init();

    if(Radio.Init() != OK) Led.StartSequence(lsqFailure);
    else Led.StartSequence(lsqStart);

    // Timers
    chVTSet(&App.TmrCheck, MS2ST(RXBUF_CHECK_PERIOD_MS), TmrCheckCallback, nullptr);
    chVTSet(&App.TmrIndication, MS2ST(INDICATION_PERIOD_MS), TmrIndicationCallback, nullptr);

    // Main cycle
    App.ITask();
}


__attribute__ ((__noreturn__))
void App_t::ITask() {
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
        // ==== Uart cmd ====
        if(EvtMsk & EVTMSK_UART_NEW_CMD) {
            OnUartCmd(&Uart);
            Uart.SignalCmdProcessed();
        }

        // ==== Rx buf check ====
        if(EvtMsk & EVTMSK_RX_BUF_CHECK) {
            RxTable.Clear();
//            uint32_t RID=0;
//            while(Radio.IdBuf.Get(&RID) == OK) RxTable.AddID(RID);
//            Uart.Printf("\rCnt = %u", RxTable.Cnt);
            if     (RxTable.Cnt == 1) VibroChunk = vsqSingle;
            else if(RxTable.Cnt == 2) VibroChunk = vsqPair;
            else if(RxTable.Cnt  > 2) VibroChunk = vsqMany;
            else VibroChunk = nullptr;
        }

        // ==== Indication ====
        if(EvtMsk & EVTMSK_INDICATION) {
//            if(VibroChunk != nullptr) Vibro.StartSequence(VibroChunk);
//            Uart.Printf("\rInd");
        }
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
