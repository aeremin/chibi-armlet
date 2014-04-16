/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
#include "cmd_uart.h"
#include "pill_mgr.h"
#include "peripheral.h"
#include "evt_mask.h"
#include "eestore.h"
#include "radio_lvl1.h"

App_t App;

#define UART_RPL_BUF_SZ     36
static uint8_t SBuf[UART_RPL_BUF_SZ];

// Table of colors depending on type
#define DEVICETYPE_BLINK_T_MS   999

const LedChunk_t TypeColorTbl[8] = {
        {clBlack,   DEVICETYPE_BLINK_T_MS, ckStop}, // dtNothing
        {clYellow,  DEVICETYPE_BLINK_T_MS, ckStop}, // dtFieldWeak
        {clGreen,   DEVICETYPE_BLINK_T_MS, ckStop}, // dtFieldNature
        {clRed,     DEVICETYPE_BLINK_T_MS, ckStop}, // dtFieldStrong
        {clCyan,    DEVICETYPE_BLINK_T_MS, ckStop}, // dtXtraNormal
        {clBlue,    DEVICETYPE_BLINK_T_MS, ckStop}, // dtXtraWeak
        {clMagenta, DEVICETYPE_BLINK_T_MS, ckStop}, // dtUfo
        {clWhite,   DEVICETYPE_BLINK_T_MS, ckStop}  // dtDetector
};

class LvlSumm_t {
private:
    int32_t tm, Sum, IThreshold;
public:
    int32_t Output;
    void Init(int32_t Threshold) {
        IThreshold = Threshold;
        tm = RX_LVL_TOP - IThreshold;
        Sum = tm;
    }
    void ProcessValue(int32_t Value) {
        if(Value < IThreshold) return;     // Ignore too low values
        Value -= IThreshold;
//        Uart.Printf("a) %d %d\r", Value, Sum);
        Sum = (Sum * (tm - Value)) / tm;
//        Uart.Printf("b) %d %d\r", Value, Sum);
    }
    int32_t CalculateLevel(int32_t Normalizer) {
        Output = 0;
        int32_t tmp = (tm - Sum - 1) * Normalizer;
//        Uart.Printf("B9) %d\r", tmp);
        if(tmp >= 0) Output = tmp / tm + 1;
        return Output;
    }
};

static LvlSumm_t LvlS[3];   // Three types of fields

#if 1 // ================================ Pill =================================
void App_t::IPillHandler() {
    // Read med
    if(PillMgr.Read(PILL_I2C_ADDR, (uint8_t*)&Pill, sizeof(Pill_t)) != OK) return;
    Uart.Printf("Pill: %u, %u\r", Pill.TypeID, Pill.DeviceType);
    if((Pill.TypeID == PILL_TYPEID_SETTYPE) and (Pill.DeviceType >= 1) and (Pill.DeviceType <= 7)) {
        SetType(Pill.DeviceType);
        Beeper.Beep(BeepPillOk);
    } // if pill ok
    // Will be here in case of bad pill
    else Beeper.Beep(BeepPillBad);
//    Led.StartBlink(LedPillBad);
//    chThdSleep(2007);    // Let indication to complete
}

#endif

#if 1 // ============================ Timers ===================================
static VirtualTimer ITmrPillCheck;
void TmrPillCheckCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_PILL_CHECK);
    chVTSetI(&ITmrPillCheck, MS2ST(TM_PILL_CHECK_MS),    TmrPillCheckCallback, nullptr);
    chSysUnlockFromIsr();
}
#endif

#if 1 // ========================= Application =================================
__attribute__((noreturn))
void App_t::ITask() {
    chRegSetThreadName("App");
    bool PillConnected = false;
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
        // ==== Check pill ====
        if(EvtMsk & EVTMSK_PILL_CHECK) {
            // Check if new connection occured
            if(PillMgr.CheckIfConnected(PILL_I2C_ADDR) == OK) {
                if(!PillConnected) {
                    PillConnected = true;
                    App.IPillHandler();
                }
            }
            else PillConnected = false;
        } // if EVTMSK_PILL_CHECK

        // ==== RX table ====
        if(EvtMsk & EVTMSK_RX_TABLE_READY) if(RxTable.PTable->Size != 0) ITableHandler();

    } // while 1
}


void App_t::ITableHandler() {
    // Convert dBm to values 1...1000
//    RxTable.Print();
    RxTable.dBm2Percent();
    RxTable.Print();

    if(Type == dtDetector) {
        TypeLvl_t TopTL;
        // Find top level and type
        for(uint32_t i=0; i<RxTable.PTable->Size; i++) {
            Row_t *PRow = &RxTable.PTable->Row[i];
            if(PRow->Rssi > TopTL.Level) TopTL.Set((DeviceType_t)PRow->Type, PRow->Rssi);
        } // for
        if(TopTL.Level > 0) Demo = TopTL;
    }
    else {
        // Init signal levels with thresholds
        for(uint8_t i=0; i<3; i++) LvlS[i].Init(SnsTable[Type][i]);
        // Summ all signals by type
        for(uint32_t i=0; i<RxTable.PTable->Size; i++) {
            Row_t *PRow = &RxTable.PTable->Row[i];
            switch(PRow->Type) {
                case dtFieldWeak:   LvlS[0].ProcessValue(PRow->Rssi); break;
                case dtFieldNature: LvlS[1].ProcessValue(PRow->Rssi); break;
                case dtFieldStrong: LvlS[2].ProcessValue(PRow->Rssi); break;
                default: break;
            }
        } // for
        // Calculate output levels
        for(uint8_t i=0; i<3; i++) LvlS[i].CalculateLevel(LVL_STEPS_N);
        // ==== Find top level and type ====
        // Ignore other fields if dtFieldStrong exists
        if     (LvlS[2].Output > 0) Demo.Set(dtFieldStrong, LvlS[2].Output);
        // Otherwise, ignore dtFieldWeak if dtFieldNature exists
        else if(LvlS[1].Output > 0) Demo.Set(dtFieldNature, LvlS[1].Output);
        else if(LvlS[0].Output > 0) Demo.Set(dtFieldWeak,   LvlS[0].Output);
    } // if not detector
    Uart.Printf("DLvl=%d; DType=%u\r", Demo.Level, Demo.Type);
//    Uart.Printf("Lvl1=%u; Lvl2=%u; Lvl3=%u; Top=%u; Type=%u\r", LvlS[0].Output, LvlS[1].Output, LvlS[2].Output, TopLvl, TopType);
}

const VibroChunk_t *PVibroTable[3][4] = {
        {Brr1, Brr2, Brr3, Brr4},   // dtFieldWeak
        {Brr4, Brr5, Brr6, Brr7},   // dtFieldNature
        {Brr7, Brr8, Brr9, Brr10}   // dtFieldStrong
};

void App_t::IDemonstrate(int32_t Level, DeviceType_t AType) {
//    Uart.Printf("Lvl=%u; Type=%u\r", Level, AType);
    uint8_t TypeID = (uint8_t)AType - 1;
    Level--;    // 1...4 => 0...3
    if((TypeID > 2) or (Level > 3)) return;
    Vibro.Flinch(PVibroTable[TypeID][Level]);
}

void App_t::Init() {
    RxTable.RegisterAppThd(PThd);
    // Read device ID and type
    ID = EE.Read32(EE_DEVICE_ID_ADDR);
    uint32_t t = EE.Read32(EE_DEVICE_TYPE_ADDR);
    SetType(t);

    // Timers init
    chSysLock();
//    chVTSetI(&ITmrDose,      MS2ST(TM_DOSE_INCREASE_MS), TmrDoseCallback, nullptr);
//    chVTSetI(&ITmrDoseSave,  MS2ST(TM_DOSE_SAVE_MS),     TmrDoseSaveCallback, nullptr);
    chVTSetI(&ITmrPillCheck, MS2ST(TM_PILL_CHECK_MS),    TmrPillCheckCallback, nullptr);
    chSysUnlock();
}

uint8_t App_t::SetType(uint8_t AType) {
    if(AType > 7) return FAILURE;
    Type = (DeviceType_t)AType;
    Led.StartBlink(&TypeColorTbl[AType]);   // Blink with correct color
    // Save in EE if not equal
    uint32_t EEType = EE.Read32(EE_DEVICE_TYPE_ADDR);
    uint8_t rslt = OK;
    if(EEType != Type) rslt = EE.Write32(EE_DEVICE_TYPE_ADDR, Type);
    Uart.Printf("ID=%u; Type=%u\r", ID, Type);
    return rslt;
}
#endif

#if 1 // ======================= Command processing ============================
void Ack(uint8_t Result) { Uart.Cmd(0x90, &Result, 1); }

void UartCmdCallback(uint8_t CmdCode, uint8_t *PData, uint32_t Length) {
    uint8_t b, b2;
    uint16_t w;
    switch(CmdCode) {
        case CMD_PING: Ack(OK); break;

#if 1 // ==== ID and type ====
        case CMD_SET_ID:
            if(Length == 2) {
                w = (PData[0] << 8) | PData[1];
                App.ID = w;
                b = App.EE.Write32(EE_DEVICE_ID_ADDR, App.ID);
                Ack(b);
            }
            else Ack(CMD_ERROR);
            break;
        case CMD_GET_ID:
            SBuf[0] = (App.ID >> 8) & 0xFF;
            SBuf[1] = App.ID & 0xFF;
            Uart.Cmd(RPL_GET_ID, SBuf, 2);
            break;

        case CMD_SET_TYPE:
            if(Length == 1) Ack(App.SetType(PData[0]));
            else Ack(CMD_ERROR);
            break;
        case CMD_GET_TYPE:
            SBuf[0] = (uint8_t)App.Type;
            Uart.Cmd(RPL_GET_TYPE, SBuf, 1);
            break;
#endif

#if 1 // ==== Pills ====
        case CMD_PILL_STATE:
            b = PData[0];   // Pill address
            if(b <= 7) {
                SBuf[1] = PillMgr.CheckIfConnected(PILL_I2C_ADDR);
                SBuf[0] = b;
                Uart.Cmd(RPL_PILL_STATE, SBuf, 2);
            }
            else Ack(CMD_ERROR);
            break;
        case CMD_PILL_WRITE:
            b = PData[0];   // Pill address
            if(b <= 7) {
                SBuf[0] = b;    // Pill address
                SBuf[1] = PillMgr.Write(PILL_I2C_ADDR, &PData[1], Length-1); // Write result
                Uart.Cmd(RPL_PILL_WRITE, SBuf, 2);
            }
            else Ack(CMD_ERROR);
            break;
        case CMD_PILL_READ:
            b = PData[0];           // Pill address
            b2 = PData[1];          // Data size to read
            if(b2 > (UART_RPL_BUF_SZ-2)) b2 = (UART_RPL_BUF_SZ-2);  // Check data size
            if(b <= 7) {
                SBuf[0] = b;                                            // Pill address
                SBuf[1] = PillMgr.Read(PILL_I2C_ADDR, &SBuf[2], b2);    // Read result
                if(SBuf[1] == OK) Uart.Cmd(RPL_PILL_READ, SBuf, b2+2);
                else Uart.Cmd(RPL_PILL_READ, SBuf, 2);
            }
            else Ack(CMD_ERROR);
            break;
#endif
        default: Ack(CMD_ERROR); break;
    } // switch
}
#endif
