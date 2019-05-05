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

// EEAddresses
#define EE_ADDR_DEVICE_ID   0

int32_t ID;
static uint8_t ISetID(int32_t NewID);
void ReadIDfromEE();

// ==== Periphery ====
//Vibro_t VibroMotor {VIBRO_SETUP};
#if BEEPER_ENABLED
Beeper_t Beeper {BEEPER_PIN};
#endif

LedRGB_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN };

// ==== Timers ====
//static TmrKL_t TmrEverySecond {TIME_MS2I(1000), evtIdEverySecond, tktPeriodic};
//static TmrKL_t TmrRxTableCheck {MS2ST(2007), evtIdCheckRxTable, tktPeriodic};
//static int32_t TimeS;
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
    ReadIDfromEE();
    Printf("\r%S %S; ID=%u\r", APP_NAME, XSTRINGIFY(BUILD_TIME), ID);
//    Uart.Printf("ID: %X %X %X\r", GetUniqID1(), GetUniqID2(), GetUniqID3());
//    if(Sleep::WasInStandby()) {
//        Uart.Printf("WasStandby\r");
//        Sleep::ClearStandbyFlag();
//    }
    Clk.PrintFreqs();
//    RandomSeed(GetUniqID3());   // Init random algorythm with uniq ID

    Led.Init();
//    Led.SetupSeqEndEvt(chThdGetSelfX(), EVT_LED_SEQ_END);
//    VibroMotor.Init();
#if BEEPER_ENABLED // === Beeper ===
    Beeper.Init();
    Beeper.StartOrRestart(bsqBeepBeep);
#endif
#if BUTTONS_ENABLED
    SimpleSensors::Init();
#endif
//    Adc.Init();

#if PILL_ENABLED // === Pill ===
    i2c1.Init();
    PillMgr.Init();
#endif

    // Init clicker
    rccEnableTIM2(FALSE);
    TIM2->CR1 = TIM_CR1_OPM;
    TIM2->ARR = 22;
    TIM2->CCMR1 = (0b111 << 12);
    TIM2->CCER = TIM_CCER_CC2E;
    uint32_t tmp = TIM2->ARR * 1000;
    tmp = Clk.APB1FreqHz / tmp;
    if(tmp != 0) tmp--;
    TIM2->PSC = (uint16_t)tmp;
    TIM2->CCR2 = 20;
    PinSetupAlterFunc(GPIOB, 3, omPushPull, pudNone, AF1);

    // ==== Radio ====
    if(Radio.Init() == retvOk) Led.StartOrRestart(lsqStart);
    else Led.StartOrRestart(lsqFailure);
    chThdSleepMilliseconds(1008);

    // Main cycle
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdEverySecond:
                break;

            case evtIdDamagePkt:
                TMR_ENABLE(TIM2);
                Led.StartOrRestart(lsqBlinkR);
                break;

#if BUTTONS_ENABLED
            case evtIdButtons:
//                Printf("Btn %u\r", Msg.BtnEvtInfo.BtnID);
                if(Msg.BtnEvtInfo.Type == beShortPress) {
                    SendEventSM(BUTTON_PRESSED_SIG, 0, 0);
                }
                else if(Msg.BtnEvtInfo.Type == beLongCombo and Msg.BtnEvtInfo.BtnCnt == 3) {
                    Printf("Combo\r");
                    SendEventSM(BUTTON_LONG_PRESSED_SIG, 0, 0);
                }
                break;
#endif

#if PILL_ENABLED // ==== Pill ====
            case evtIdPillConnected:
                Printf("Pill: %u\r", PillMgr.Pill.DWord32);
                switch(PillMgr.Pill.DWord32) {
                    case 0: SendEventSM(PILL_RESET_SIG, 0, 0); break;
                    case 1: SendEventSM(PILL_MUTANT_SIG, 0, 0); break;
                    case 2: SendEventSM(PILL_IMMUNE_SIG, 0, 0); break;
                    case 3: SendEventSM(PILL_HP_DOUBLE_SIG, 0, 0); break;
                    case 4: SendEventSM(PILL_HEAL_SIG, 0, 0); break;
                    case 5: SendEventSM(PILL_SURGE_SIG , 0, 0); break;
                    default: break;
                }
                break;

            case evtIdPillDisconnected:
                Printf("Pill disconn\r");
                SendEventSM(PILL_REMOVED_SIG, 0, 0);
//                Led.StartOrRestart(lsqNoPill);
                break;
#endif

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

    else if(PCmd->NameIs("GetID")) PShell->Reply("ID", ID);

    else if(PCmd->NameIs("SetID")) {
        if(PCmd->GetNext<int32_t>(&ID) != retvOk) { PShell->Ack(retvCmdError); return; }
        uint8_t r = ISetID(ID);
        RMsg_t msg;
        msg.Cmd = R_MSG_SET_CHNL;
        msg.Value = ID2RCHNL(ID);
        Radio.RMsgQ.SendNowOrExit(msg);
        PShell->Ack(r);
    }


#if 0 // === Pill ===
    else if(PCmd->NameIs("ReadPill")) {
        int32_t DWord32;
        uint8_t Rslt = PillMgr.Read(0, &DWord32, 4);
        if(Rslt == retvOk) {
            PShell->Print("Read %d\r\n", DWord32);
        }
        else PShell->Ack(retvFail);
    }

    else if(PCmd->NameIs("WritePill")) {
        int32_t DWord32;
        if(PCmd->GetNext<int32_t>(&DWord32) == retvOk) {
            uint8_t Rslt = PillMgr.Write(0, &DWord32, 4);
            PShell->Ack(Rslt);
        }
        else PShell->Ack(retvCmdError);
    }
#endif

    else PShell->Ack(retvCmdUnknown);
}
#endif


#if 1 // =========================== ID management =============================
void ReadIDfromEE() {
    ID = EE::Read32(EE_ADDR_DEVICE_ID);  // Read device ID
    if(ID < ID_MIN or ID > ID_MAX) {
        Printf("\rUsing default ID\r");
        ID = ID_DEFAULT;
    }
}

uint8_t ISetID(int32_t NewID) {
    if(NewID < ID_MIN or NewID > ID_MAX) return retvFail;
    uint8_t rslt = EE::Write32(EE_ADDR_DEVICE_ID, NewID);
    if(rslt == retvOk) {
        ID = NewID;
        Printf("New ID: %u\r", ID);
        return retvOk;
    }
    else {
        Printf("EE error: %u\r", rslt);
        return retvFail;
    }
}
#endif
