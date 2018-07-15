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

LedHSV_t LedHsv { LED_R_PIN, LED_G_PIN, LED_B_PIN };
static LedHSVChunk_t lsqFlicker[] = {
        {csSetup, 99, hsvRed},
        {csSetup, 99, hsvBlack},
        {csEnd}
};

Timer_t SyncTmr(TIM9);
uint16_t GetTimerArr(uint32_t Period);

#define BLINK_PERIOD_MAX_S  100

Mode_t Mode = modeOff;
static uint16_t ClrH = 0;
static uint16_t Period = BLINK_PERIOD_MAX_S+1;
bool IsFlickering = false;

void ProcessCmd(Mode_t NewMode, uint16_t NewClrH, uint16_t NewPeriod);
uint16_t GetRandomClrH() {
    int16_t H;
    int16_t OldH = lsqFlicker[0].Color.H;
    do {
        H = Random::Generate(0, 360);
    } while(ABS(H - OldH) < 36);
    return H;
}

void ProcessTmrUpdate();
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

    Random::Seed(GetUniqID3());

    LedHsv.Init();

    if(Radio.Init() == retvOk) LedHsv.StartOrRestart(lsqHsvStart);
    else LedHsv.StartOrRestart(lsqHsvFailure);

    // Setup sync timer
    SyncTmr.Init();
    SyncTmr.SetTopValue(63000);
    SyncTmr.SetupPrescaler(1000);
    SyncTmr.EnableIrqOnUpdate();
    SyncTmr.EnableIrq(TIM9_IRQn, IRQ_PRIO_LOW);

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

            case evtIdNewRadioCmd:
//                Printf("radio\r");
                ProcessCmd((Mode_t)Msg.w16[0], Msg.w16[1], Msg.w16[2]);
                break;

            case evtIdSyncTmrUpdate: ProcessTmrUpdate(); break;

            default: Printf("Unhandled Msg %u\r", Msg.ID); break;
        } // switch
    } // while true
}

void ProcessCmd(Mode_t NewMode, uint16_t NewClrH, uint16_t NewPeriod) {
    IsFlickering = (NewPeriod <= BLINK_PERIOD_MAX_S);
    // Process mode
    if(Mode != NewMode) {
        Mode = NewMode;
        LedHsv.Stop();
        SyncTmr.SetCounter(0);
        if(Mode == modeOff) SyncTmr.Disable();
        else {
            // Timer
            if(IsFlickering) {
                SyncTmr.Enable();
                SyncTmr.SetTopValue(GetTimerArr(Period));
            }
            else SyncTmr.Disable();
            // Led
            ClrH = NewClrH;
            if(Mode == modeRandom and IsFlickering) ClrH = GetRandomClrH();
            if(IsFlickering) {
                LedHsv.SetColorAndMakeCurrent(ColorHSV_t(ClrH, 100, 0));
                lsqFlicker[0].Color.FromHSV(ClrH, 100, 100);
                lsqFlicker[1].Color.FromHSV(ClrH, 100, 0);    // Make it black
                LedHsv.StartOrRestart(lsqFlicker);
            }
            else { // Stay off if random and not flickering
                if(Mode != modeRandom) LedHsv.SetColorAndMakeCurrent(ColorHSV_t(ClrH, 100, 100));
            }
        } // if modeOff
    } // if mode changed
    // Mode unchanged
    else {
        if(Mode == modeOff) return; // Nothing to do here
        // Period
        if(NewPeriod != Period) {
            Printf("Period changed\r");
            Period = NewPeriod;
            LedHsv.Stop();
            if(IsFlickering) {
                // Timer
                SyncTmr.SetCounter(0);
                SyncTmr.SetTopValue(GetTimerArr(Period));
                SyncTmr.Enable();
                // lsq smooth
                lsqFlicker[0].Value = Period * 18;
                lsqFlicker[1].Value = lsqFlicker[0].Value;
                LedHsv.StartOrRestart(lsqFlicker);
            }
            else SyncTmr.Disable();
        }

        // Color (if not Random)
        if(NewMode == modeAsync or NewMode == modeSync) {
            ClrH = NewClrH;
            if(IsFlickering) {
                lsqFlicker[0].Color.FromHSV(ClrH, 100, 100);
                lsqFlicker[1].Color.FromHSV(ClrH, 100, 0);    // Make it black
                LedHsv.SetCurrentH(ClrH);
            }
            else LedHsv.SetColorAndMakeCurrent(ColorHSV_t(ClrH, 100, 100));
        }
    }
}

void ProcessTmrUpdate() {
    if(!IsFlickering) {
        SyncTmr.Disable();
        return;
    }
    switch(Mode) {
        case modeOff: SyncTmr.Disable(); break;

        case modeSync:
            LedHsv.StartOrRestart(lsqFlicker);
            break;

        case modeAsync: {
            uint16_t NewTopValue = GetTimerArr(Period) + Random::Generate(0, 540);
            SyncTmr.SetCounter(0);
            SyncTmr.SetTopValue(NewTopValue);
            LedHsv.StartOrRestart(lsqFlicker);
            Printf("%u %u\r", GetTimerArr(Period), NewTopValue);
        }
        break;

        case modeRandom: {
            uint16_t NewTopValue = GetTimerArr(Period) + Random::Generate(0, 540);
            SyncTmr.SetCounter(0);
            SyncTmr.SetTopValue(NewTopValue);
            ClrH = GetRandomClrH();
            lsqFlicker[0].Color.FromHSV(ClrH, 100, 100);
            lsqFlicker[1].Color.FromHSV(ClrH, 100, 0);
            LedHsv.SetColorAndMakeCurrent(ColorHSV_t(ClrH, 100, 0));
            LedHsv.StartOrRestart(lsqFlicker);
        }
        break;
    }
}

uint16_t GetTimerArr(uint32_t Period) {
    if     (Period >= 80) return Period * 114 + 11;
    else if(Period >= 60) return Period * 115 + 11;
    else if(Period >= 40) return Period * 116 + 11;
    else if(Period >= 30) return Period * 117 + 11;
    else if(Period >= 20) return Period * 119 + 11;
    else if(Period >= 15) return Period * 121 + 11;
    else if(Period >= 10) return Period * 125 + 11;
    else if(Period >= 7) return Period * 130 + 11;
    else if(Period >= 4) return Period * 155 + 11;
    else if(Period == 3) return 534;
    else if(Period == 2) return 469;
    else return 412;
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

// IRQ
extern "C"
void VectorA4() {
    CH_IRQ_PROLOGUE();
    chSysLockFromISR();
    SyncTmr.ClearIrqPendingBit();
    EvtQMain.SendNowOrExitI(EvtMsg_t(evtIdSyncTmrUpdate));
    chSysUnlockFromISR();
    CH_IRQ_EPILOGUE();
}
