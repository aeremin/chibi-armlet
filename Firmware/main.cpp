#include "board.h"
#include "led.h"
#include "vibro.h"
#include "beeper.h"
#include "Sequences.h"
#include "radio_lvl1.h"
#include "kl_i2c.h"
#include "kl_lib.h"
#include "pill.h"
#include "pill_mgr.h"
#include "MsgQ.h"
#include "main.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};
static void ITask();
static void OnCmd(Shell_t *PShell);

LedRGB_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN };

// ==== Timers ====
static TmrKL_t TmrEverySecond {TIME_MS2I(999), evtIdEverySecond, tktPeriodic};
static uint32_t AppearTimeout = 0;
static uint32_t TableCheckTimeout = CHECK_PERIOD_S;
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
    Clk.SetMSI4MHz();
    Clk.UpdateFreqValues();

    // === Init OS ===
    halInit();
    chSysInit();
    EvtQMain.Init();

    // ==== Init hardware ====
    Uart.Init();
    Printf("\r%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));
    Clk.PrintFreqs();

    Led.Init();

    // ==== Radio ====
    if(Radio.Init() == retvOk) Led.StartOrRestart(lsqStart);
    else Led.StartOrRestart(lsqFailure);
    chThdSleepMilliseconds(1008);

    TmrEverySecond.StartOrRestart();

    // Main cycle
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdEverySecond:
//                Printf("Second\r");
                // Check if time to disappear
                if(AppearTimeout > 0) {
                    AppearTimeout--;
                    if(AppearTimeout == 0) Led.StartOrRestart(lsqDisappear);
                }
                // Check if time to check RxTable
                if(TableCheckTimeout > 0) {
                    TableCheckTimeout--;
                    if(TableCheckTimeout == 0) {
                        TableCheckTimeout = CHECK_PERIOD_S;
                        // Check table
                        if(Radio.RxTable.GetCount() > 0) {
                            Printf("Cnt=%u\r", Radio.RxTable.GetCount());
// #if GREEN_AND_WHITE
                            // Presence of ID=2 means White is here
//                            if(Radio.RxTable.IDPresents(2)) {
//                                Led.StartOrRestart(lsqAppearWhite);
//                            }
//                            else Led.StartOrRestart(lsqAppearGreen);
//                            AppearTimeout = APPEAR_DURATION;
//#else
                            // Different IDs give different colours
                            if(Radio.RxTable.IDPresents(1)) {
                                Led.StartOrRestart(lsqAppearGreen);
                            } else if (Radio.RxTable.IDPresents(2)) {
                                Led.StartOrRestart(lsqAppearRed);
                            } else if (Radio.RxTable.IDPresents(3)) {
                                Led.StartOrRestart(lsqAppearBlue);
                            } else if (Radio.RxTable.IDPresents(4)) {
                                Led.StartOrRestart(lsqAppearYellow);
                            } else if (Radio.RxTable.IDPresents(5)) {
                                Led.StartOrRestart(lsqAppearMagenta);
                            } else if (Radio.RxTable.IDPresents(6)) {
                                Led.StartOrRestart(lsqAppearCyan);
                            } else {
                                Led.StartOrRestart(lsqAppearWhite);
                            }
                            AppearTimeout = APPEAR_DURATION;
//#endif
                        }
                        Radio.RxTable.Clear();
                    }
                }
                break;

            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;
            default: Printf("Unhandled Msg %u\r", Msg.ID); break;
        } // Switch
    } // while true
} // ITask()


#if 1 // ================= Command processing ====================
void OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
//    Uart.Printf("%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(retvOk);
    }
    else if(PCmd->NameIs("Version")) PShell->Print("%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));

    else PShell->Ack(retvCmdUnknown);
}
#endif
