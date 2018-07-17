#include "kl_lib.h"
#include "ch.h"
#include "hal.h"
#include "uart.h"
#include "radio_lvl1.h"
//#include "vibro.h"
#include "led.h"
#include "Sequences.h"
#include "main.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};
static void ITask();
static void OnCmd(Shell_t *PShell);

LedRGB_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN };

TmrKL_t TmrEverySecond {MS2ST(999), evtIdEverySecond, tktPeriodic};
static uint32_t AppearTimeout = 0;
static uint32_t TableCheckTimeout = CHECK_PERIOD_S;
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V2);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();
    EvtQMain.Init();

    // ==== Init Hard & Soft ====
    Uart.Init();
    Uart.StartRx();
    Printf("\r%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));
    Clk.PrintFreqs();

    Led.Init();

    TmrEverySecond.StartOrRestart();

    if(Radio.Init() == retvOk) Led.StartOrRestart(lsqStart);
    else Led.StartOrRestart(lsqFailure);

    // Main cycle
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;

            case evtIdEverySecond:
//                Printf("Second\r");
                if(AppearTimeout > 0) {
                    AppearTimeout--;
                    if(AppearTimeout == 0) Led.StartOrRestart(lsqIdle);
                }

                if(TableCheckTimeout > 0) {
                    TableCheckTimeout--;
                    if(TableCheckTimeout == 0) {
                        TableCheckTimeout = CHECK_PERIOD_S;
                        // Check table
//                        Printf("TblCnt: %u\r", Radio.RxTable.GetCount());
                        if(Radio.RxTable.GetCount() > 0) {
                            AppearTimeout = APPEAR_DURATION;
                            Led.StartOrContinue(lsqAppear);
                            Radio.RxTable.Clear();
                        }
                    }
                }
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
    else if(PCmd->NameIs("Version")) PShell->Printf("%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));


    else PShell->Ack(retvCmdUnknown);
}
