/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
#include "cmd_uart.h"
#include "pill.h"
#include "peripheral.h"
#include "sequences.h"
#include "evt_mask.h"
#include "eestore.h"

App_t App;
static uint8_t SBuf[252];
static Thread *PAppThd;

static void SignalError(const char* S) {
    Uart.Printf(S);
}

#if 1 // ================================ Dose =================================
enum HealthState_t {hsNone=0, hsGreen, hsYellow, hsRedSlow, hsRedFast, hsDeath};
class Dose_t {
private:
    uint32_t IDose;
    EEStore_t EE;   // EEPROM storage for dose
    void ChangeIndication() {
        Beeper.Stop();
        Led.StopBlink();
        switch(State) {
            case hsDeath:
                Led.SetColor(clRed);
                Beeper.Beep(BeepDeath);
                break;
            case hsRedFast:
                Led.StartBlink(LedRedFast);
                Beeper.Beep(BeepRedFast);
                break;
            case hsRedSlow:
                Led.StartBlink(LedRedSlow);
                Beeper.Beep(BeepBeep);
                break;
            case hsYellow:
                Led.StartBlink(LedYellow);
                Beeper.Beep(BeepBeep);
                break;
            case hsGreen:
                Led.StartBlink(LedGreen);
                Beeper.Beep(BeepBeep);
                break;
            default: break;
        } // switch
    }
    void ConvertDoseToState() {
        if     (IDose >= DOSE_RED_END)  State = hsDeath;
        else if(IDose >= DOSE_RED_FAST) State = hsRedFast;
        else if(IDose >= DOSE_RED_SLOW) State = hsRedSlow;
        else if(IDose >= DOSE_YELLOW)   State = hsYellow;
        else                            State = hsGreen;
    }
public:
    HealthState_t State;
    void Set(uint32_t ADose) {
        IDose = ADose;
        HealthState_t OldState = State;
        ConvertDoseToState();
        if(State != OldState) ChangeIndication();
    }
    uint32_t Get() { return IDose; }
    void Increase(uint32_t Amount) {
        uint32_t Dz = IDose;
        if((Dz + Amount) > DOSE_RED_END) Dz = DOSE_RED_END;
        else Dz += Amount;
        Set(Dz);
    }
    // Save if changed
    uint8_t Save() {
        uint32_t OldDose = 0;
        if(EE.Get(&OldDose) == OK) {
            if(OldDose == IDose) return OK;
        }
        return EE.Put(&IDose);
    }
    // Try load from EEPROM, set 0 if failed
    void Load() {
        uint32_t FDose = 0;
        EE.Get(&FDose);     // Try to read
        Set(FDose);
    }
};
static Dose_t Dose;
#endif

#if 1 // ================================ Pill =================================
struct Med_t {
    uint16_t CureID;
    uint16_t Charge;
} __attribute__ ((__packed__));
static Med_t Med;
#endif

#if 1 // ============================ Timers ===================================
static VirtualTimer ITmrDose, ITmrDoseSave, ITmrPillCheck;
void TmrDoseCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(PAppThd, EVTMSK_DOSE_INC);
    chVTSetI(&ITmrDose,      MS2ST(TM_DOSE_INCREASE_MS), TmrDoseCallback, nullptr);
    chSysUnlockFromIsr();
}
void TmrDoseSaveCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(PAppThd, EVTMSK_DOSE_STORE);
    chVTSetI(&ITmrDoseSave,  MS2ST(TM_DOSE_SAVE_MS),     TmrDoseSaveCallback, nullptr);
    chSysUnlockFromIsr();
}
void TmrPillCheckCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(PAppThd, EVTMSK_PILL_CHECK);
    chVTSetI(&ITmrPillCheck, MS2ST(TM_PILL_CHECK_MS),    TmrPillCheckCallback, nullptr);
    chSysUnlockFromIsr();
}

#endif

#if 1 // ========================= Application =================================
static WORKING_AREA(waAppThread, 256);
__attribute__((noreturn))
static void AppThread(void *arg) {
    chRegSetThreadName("App");
    uint32_t EvtMsk;
    while(1) {
        EvtMsk = chEvtWaitAny(ALL_EVENTS);
        // ==== Process dose ====
        if(EvtMsk & EVTMSK_DOSE_INC) {
            Dose.Increase(1);
        }

        // ==== Store dose ====
        if(EvtMsk & EVTMSK_DOSE_STORE) {
            if(Dose.Save() != OK) SignalError("Save Fail\r");
        }

        // ==== Check pills ====
        if(EvtMsk & EVTMSK_PILL_CHECK) {
            PillChecker();
            if(PillsHaveChanged) {  // Will be reset at PillChecker
                Beeper.Beep(BeepPillBad);
                // Read med
                if(Pill[0].Connected) {
                    Pill[0].Read((uint8_t*)&Med, sizeof(Med_t));
                    Uart.Printf("Pill: %u, %u\r", Med.CureID, Med.Charge);
                }
            } // if pill changed
        }
    } // while 1
}

void App_t::Init() {
    Dose.Load();
    Uart.Printf("Dose = %u\r", Dose.Get());
    PAppThd = chThdCreateStatic(waAppThread, sizeof(waAppThread), NORMALPRIO, (tfunc_t)AppThread, NULL);
    // Timers init
    chSysLock();
    chVTSetI(&ITmrDose,      MS2ST(TM_DOSE_INCREASE_MS), TmrDoseCallback, nullptr);
    chVTSetI(&ITmrDoseSave,  MS2ST(TM_DOSE_SAVE_MS),     TmrDoseSaveCallback, nullptr);
    chVTSetI(&ITmrPillCheck, MS2ST(TM_PILL_CHECK_MS),    TmrPillCheckCallback, nullptr);
    chSysUnlock();
}
#endif

#if 1 // ======================= Command processing ============================
void Ack(uint8_t Result) { Uart.Cmd(0x90, &Result, 1); }

void UartCmdCallback(uint8_t CmdCode, uint8_t *PData, uint32_t Length) {
    uint8_t b, b2;
    uint32_t w, *p;
    switch(CmdCode) {
        case CMD_PING: Ack(OK); break;

        case 0x51:  // GetID
            Uart.Printf("ID=%u\r", App.ID);
            break;

        case 0x52:  // SetID
            App.ID = *((uint16_t*)PData);
            Uart.Printf("New ID=%u\r", App.ID);
            break;

        // ==== Pills ====
        case CMD_PILL_STATE:
            b = PData[0];   // Pill address
            if(b <= 7) SBuf[1] = Pill[b].CheckIfConnected();
            SBuf[0] = b;
            Uart.Cmd(RPL_PILL_STATE, SBuf, 2);
            break;
        case CMD_PILL_WRITE:
            b = PData[0];
            if(b <= 7) SBuf[1] = Pill[b].Write(&PData[1], Length-1);
            SBuf[0] = b;
            Uart.Cmd(RPL_PILL_WRITE, SBuf, 2);
            break;
        case CMD_PILL_READ:
            b = PData[0];           // Pill address
            b2 = PData[1];          // Data size to read
            if(b2 > 250) b2 = 250;  // Check data size
            if(b <= 7) SBuf[1] = Pill[b].Read(&SBuf[2], b2);
            SBuf[0] = b;
            if(SBuf[1] == OK) Uart.Cmd(RPL_PILL_READ, SBuf, b2+2);
            else Uart.Cmd(RPL_PILL_READ, SBuf, 2);
            break;

        // ==== Dose ====
        case CMD_DOSE_GET:
            p = (uint32_t*)SBuf;
            *p = Dose.Get();
            Uart.Cmd(RPL_DOSE_GET, SBuf, 4);
            break;
        case CMD_DOSE_SET:
            w = *((uint32_t*)PData);
            if(w <= DOSE_RED_END) {
                Dose.Set(w);
                Ack(OK);
            }
            else Ack(FAILURE);
            break;

        default: break;
    } // switch
}
#endif
