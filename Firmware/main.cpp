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

void SetupActiveColor(bool AuleIsHere, bool OromeIsHere);
void ShowKind();

static enum Kind_t {
    kndOromeY=0, kndOromeG=1, kndOromeW=2, kndOromeB=3,
    kndOromeBAuleR=4, kndOromeBAuleY=5, kndOromeBAuleO=6
} Kind;

static LedRGBChunk_t lsqStart[] = {
        {csSetup, 0, clBlue},   // Color is dummy
        {csWait, 720},
        {csSetup, 0, clYellow}, // Color is dummy
        {csWait, 720},
        {csSetup, 0, clBlack},
        {csEnd}
};
static LedRGBChunk_t lsqAppear[] = {
        {csSetup, 360, clBlack}, // Color is dummy
        {csEnd}
};
static const LedRGBChunk_t lsqDisappear[] = {
        {csSetup, 360, clBlack},
        {csEnd}
};

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

//    uint32_t tmp = EE::Read32(EE_ADDR_KIND);
//    if(tmp > 6) tmp = 0;
    Kind = kndOromeB;//(Kind_t)tmp;

    Led.Init();

    TmrEverySecond.StartOrRestart();

    if(Radio.Init() == retvOk) ShowKind();
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
                    if(AppearTimeout == 0) Led.StartOrRestart(lsqDisappear);
                }

                if(TableCheckTimeout > 0) {
                    TableCheckTimeout--;
                    if(TableCheckTimeout == 0) {
                        TableCheckTimeout = CHECK_PERIOD_S;
                        // Check table
                        if(Radio.RxTable.GetCount() > 0) {
                            Printf("Cnt=%u\r", Radio.RxTable.GetCount());
                            // Presence of ID=0 means Aule is here. Absence means Orome only.
                            bool AuleIsHere = Radio.RxTable.IDPresents(0);
                            bool OromeIsHere = (!AuleIsHere) or (Radio.RxTable.GetCount() > 1);
                            SetupActiveColor(AuleIsHere, OromeIsHere);
                            Led.StartOrRestart(lsqAppear);
                            AppearTimeout = APPEAR_DURATION;
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
        } // switch
    } // while true
}

void SetupActiveColor(bool AuleIsHere, bool OromeIsHere) {
//    Printf("SAC Aule=%u, Orome=%u\r", AuleIsHere, OromeIsHere);
    lsqAppear[0].Color = clBlack;
    switch(Kind) {
        case kndOromeY:
            if(OromeIsHere) lsqAppear[0].Color = clYellow;
            break;

        case kndOromeG:
            if(OromeIsHere) lsqAppear[0].Color = clGreen;
            break;

        case kndOromeW:
            if(OromeIsHere) lsqAppear[0].Color = clWhite;
            break;

        case kndOromeB:
            if(OromeIsHere) lsqAppear[0].Color = clBlue;
            break;

        case kndOromeBAuleR:
            if(AuleIsHere) lsqAppear[0].Color = clRed;
            else if(OromeIsHere) lsqAppear[0].Color = clBlue;
            break;

        case kndOromeBAuleY:
            if(AuleIsHere) lsqAppear[0].Color = clYellow;
            else if(OromeIsHere) lsqAppear[0].Color = clBlue;
            break;

        case kndOromeBAuleO:
            if(AuleIsHere) lsqAppear[0].Color = clOrange;
            else if(OromeIsHere) lsqAppear[0].Color = clBlue;
    }
}

void ShowKind() {
    if(Kind <= kndOromeB) lsqStart[0].Color = clBlue;
    else lsqStart[0].Color = clRed;
    switch(Kind) {
        case kndOromeY: lsqStart[2].Color = clYellow; break;
        case kndOromeG: lsqStart[2].Color = clGreen;  break;
        case kndOromeW: lsqStart[2].Color = clWhite;  break;
        case kndOromeB: lsqStart[2].Color = clBlue;   break;
        case kndOromeBAuleR: lsqStart[2].Color = clRed; break;
        case kndOromeBAuleY: lsqStart[2].Color = clYellow; break;
        case kndOromeBAuleO: lsqStart[2].Color = clOrange; break;
    }
    Led.StartOrRestart(lsqStart);
    Printf("Kind: %u\r", Kind);
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

    else if(PCmd->NameIs("Kind")) {
        uint8_t IKind = 0;
        if(PCmd->GetNext<uint8_t>(&IKind) == retvOk) {
            if(IKind <= 6) {
                Kind = (Kind_t)IKind;
                EE::Write32(EE_ADDR_KIND, IKind);
                PShell->Ack(retvOk);
                ShowKind();
                return;
            }
        }
        PShell->Ack(retvCmdError);
    }

    else PShell->Ack(retvCmdUnknown);
}
