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

static Color_t ReadColorFromEE();
static void WriteColorToEE(Color_t AColor);

LedRGB_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN };

TmrKL_t TmrEverySecond {MS2ST(999), evtIdEverySecond, tktPeriodic};

static LedRGBChunk_t lsqIdle[] = {
        {csSetup, 360, clBlue},
        {csEnd}
};

static uint32_t AppearTimeout = 0;
static uint32_t SaveColorTimeout = 0;
static uint32_t TableCheckTimeout = CHECK_PERIOD_S;
bool IsIdle = true;
bool IsChoosenOne = false;
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
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

    lsqIdle[0].Color = ReadColorFromEE();
    Led.Init();

    // Check if choosen one
    IsChoosenOne = (bool)EE::Read32(EE_ADDR_CHOOSEN);
    if(IsChoosenOne) {
        Led.StartOrRestart(lsqChoosenOne);
        chThdSleepMilliseconds(2007);
    }

    TmrEverySecond.StartOrRestart();

    if(Radio.Init() == retvOk) Led.StartOrRestart(lsqIdle);
    else Led.StartOrRestart(lsqFailure);

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
                if(AppearTimeout > 0) {
                    AppearTimeout--;
                    if(AppearTimeout == 0) {
                        Led.StartOrRestart(lsqIdle);
                        IsIdle = true;
                    }
                }

                if(SaveColorTimeout > 0) {
                    SaveColorTimeout--;
                    if(SaveColorTimeout == 0) WriteColorToEE(lsqIdle[0].Color);
                }

                if(TableCheckTimeout > 0) {
                    TableCheckTimeout--;
                    if(TableCheckTimeout == 0) {
                        TableCheckTimeout = CHECK_PERIOD_S;
                        if(IsChoosenOne) {
                            // Check table
                            Printf("TblCnt: %u\r", Radio.RxTable.GetCount());
                            if(Radio.RxTable.GetCount() == 7) {
                                AppearTimeout = APPEAR_DURATION;
                                Led.StartOrContinue(lsqAppear);
                                IsIdle = false;
                            }
                        }
                        Radio.RxTable.Clear();
                    }
                }
                break;

            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;

            case evtIdNewRadioCmd: {
                Color_t Clr;
                Clr.DWord32 = (uint32_t)Msg.Value;
                Clr.Print();
                if(lsqIdle[0].Color != Clr) {
                    lsqIdle[0].Color = Clr;
                    SaveColorTimeout = 11;
                    if(IsIdle) Led.StartOrRestart(lsqIdle);
                }
            } break;

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

    else if(PCmd->NameIs("BeChoosen")) {
        IsChoosenOne = true;
        EE::Write32(EE_ADDR_CHOOSEN, 1);
        PShell->Ack(retvOk);
    }

    else if(PCmd->NameIs("BeOrdinary")) {
        IsChoosenOne = false;
        EE::Write32(EE_ADDR_CHOOSEN, 0);
        PShell->Ack(retvOk);
    }

    else PShell->Ack(retvCmdUnknown);
}

#if 1 // =========================== EE management =============================
Color_t ReadColorFromEE() {
    Color_t Rslt;
    Rslt.DWord32 = EE::Read32(EE_ADDR_COLOR);
    return Rslt;
}

void WriteColorToEE(Color_t AColor) {
    EE::Write32(EE_ADDR_COLOR, AColor.DWord32);
}
#endif
