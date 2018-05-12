/*
 * File:   main.cpp
 * Author: Kreyl
 * Project: Armlet2South
 *
 * Created on Feb 05, 2013, 20:27
 */

#include "kl_lib.h"
#include "ch.h"
#include "hal.h"
#include "uart.h"
//#include "radio_lvl1.h"
//#include "vibro.h"
#include "led.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
extern CmdUart_t Uart;
static void ITask();
static void OnCmd(Shell_t *PShell);

//static void ReadAndSetupMode();

// EEAddresses
#define EE_ADDR_DEVICE_ID       0
//#define EE_ADDR_HEALTH_STATE    8

int32_t ID;
//static uint8_t ISetID(int32_t NewID);
//void ReadIDfromEE();

//LedRGBwPower_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN, LED_EN_PIN };

// ==== Timers ====
//static TmrKL_t TmrEverySecond {MS2ST(1000), evtIdEverySecond, tktPeriodic};
//static TmrKL_t TmrRxTableCheck {MS2ST(2007), evtIdCheckRxTable, tktPeriodic};
//static int32_t TimeS;
#endif


//LedRGB_t Led({GPIOB, 1, TIM3, 4}, {GPIOB, 0, TIM3, 3}, {GPIOB, 5, TIM3, 2});

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();
    EvtQMain.Init();

    // ==== Init Hard & Soft ====
    Uart.Init(115200);
//    ReadIDfromEE();
    Printf("\r%S %S; ID=%u\r", APP_NAME, BUILD_TIME, ID);
    Clk.PrintFreqs();

//    Led.Init();

//    if(Radio.Init() != OK) Led.StartSequence(lsqFailure);
//    else Led.StartSequence(lsqStart);

    // Main cycle
    ITask();
}


__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdEverySecond:
//                TimeS++;
//                ReadAndSetupMode();
                break;

            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;
            default: Printf("Unhandled Msg %u\r", Msg.ID); break;
        } // switch
    } // while true
}

void OnCmd(Shell_t *PShell) {
    Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
//    Uart.Printf("%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(retvOk);
    }
    else if(PCmd->NameIs("Version")) PShell->Printf("%S %S\r", APP_NAME, BUILD_TIME);

    else if(PCmd->NameIs("GetID")) PShell->Reply("ID", ID);

//    else if(PCmd->NameIs("SetID")) {
//        if(PCmd->GetNext<int32_t>(&ID) != retvOk) { PShell->Ack(retvCmdError); return; }
////        uint8_t r = ISetID(ID);
//        RMsg_t msg;
//        msg.Cmd = R_MSG_SET_CHNL;
//        msg.Value = ID2RCHNL(ID);
//        Radio.RMsgQ.SendNowOrExit(msg);
//        PShell->Ack(r);
//    }

    else PShell->Ack(retvCmdUnknown);
}
