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

LedRGB_t Led({GPIOB, 1, TIM3, 4}, {GPIOB, 0, TIM3, 3}, {GPIOB, 5, TIM3, 2});

App_t App;

#if 1 // ============================ Timers ===================================
// Universal VirtualTimer callback
void TmrGeneralCallback(void *p) {
    chSysLockFromIsr();
    App.SignalEvtI((eventmask_t)p);
    chSysUnlockFromIsr();
}

VirtualTimer TmrSecond;

void TmrSecondCallback(void *p) {
    chSysLockFromIsr();
    App.SignalEvtI(EVTMSK_EVERY_SECOND);
    chVTSetI(&TmrSecond, MS2ST(1000), TmrSecondCallback, nullptr);
    chSysUnlockFromIsr();
}
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();

    // ==== Init Hard & Soft ====
    App.InitThread();
    Uart.Init(115200);
    Uart.Printf("\r%S; AHB=%u", APP_NAME, Clk.AHBFreqHz);

    Led.Init();
    App.ID = EE.Read32(EE_DEVICE_ID_ADDR);  // Read device ID
    if(App.ID > MAX_ID or App.ID < MIN_ID) App.ID = MIN_ID;
    Uart.Printf("\rID=%u\r", App.ID);


    if(Radio.Init() != OK) Led.StartSequence(lsqFailure);
    else Led.StartSequence(lsqStart);

    chVTSet(&TmrSecond, MS2ST(1000), TmrSecondCallback, nullptr);

    // Main cycle
    App.ITask();
}


__attribute__ ((__noreturn__))
void App_t::ITask() {
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);

        if(EvtMsk & EVTMSK_EVERY_SECOND) {
            TimeS++;
            Uart.Printf("%u\r", TimeS);
        }

#if 1   // ==== Uart cmd ====
        if(EvtMsk & EVTMSK_UART_NEW_CMD) {
            OnUartCmd(&Uart);
            Uart.SignalCmdProcessed();
        }
#endif

#if 0   // ==== Radio ====
        if(EvtMsk & EVTMSK_RADIO) {
//            chVTRestart(&TmrOff, INDICATION_TIME_MS, EVTMSK_OFF);
            if(OldClr != RcvdClr) {
                lsqFadeIn[0].Color = RcvdClr;
                OldClr = RcvdClr;
                Led.StartSequence(lsqFadeIn);
            }
        }
#endif

#if 0 // ==== Off ====
        if(EvtMsk & EVTMSK_OFF) {
            Uart.Printf("\rOff %u", chTimeNow());
            Led.StartSequence(lsqFadeOut);
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

    else if(PCmd->NameIs("GetID")) Uart.Reply("ID", ID);

    else if(PCmd->NameIs("SetID")) {
        if(PCmd->GetNextToken() != OK) { PUart->Ack(CMD_ERROR); return; }
        if(PCmd->TryConvertTokenToNumber(&dw32) != OK) { PUart->Ack(CMD_ERROR); return; }
        uint8_t r = ISetID(dw32);
        PUart->Ack(r);
    }

    else PUart->Ack(CMD_UNKNOWN);
}

uint8_t App_t::ISetID(int32_t NewID) {
    if(NewID > MAX_ID or NewID < MIN_ID) return FAILURE;
    uint8_t rslt = EE.Write32(EE_DEVICE_ID_ADDR, NewID);
    if(rslt == OK) {
        ID = NewID;
        Uart.Printf("New ID: %u\r", ID);
        return OK;
    }
    else {
        Uart.Printf("EE error: %u\r", rslt);
        return FAILURE;
    }
}
