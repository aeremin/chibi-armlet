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
enum DoIndication_t {diUsual, diAlwaysIndicate, diNeverIndicate};
class Dose_t {
private:
    uint32_t IDose;
    EEStore_t EE;   // EEPROM storage for dose
    void ConvertDoseToState() {
        if     (IDose >= DOSE_TOP)      State = hsDeath;
        else if(IDose >= DOSE_RED_FAST) State = hsRedFast;
        else if(IDose >= DOSE_RED)      State = hsRedSlow;
        else if(IDose >= DOSE_YELLOW)   State = hsYellow;
        else                            State = hsGreen;
    }
public:
    HealthState_t State;
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
    void Set(uint32_t ADose, DoIndication_t DoIndication) {
        IDose = ADose;
        HealthState_t OldState = State;
        ConvertDoseToState();
        if((DoIndication == diAlwaysIndicate) or ((State != OldState) and (DoIndication == diUsual))) ChangeIndication();
    }
    uint32_t Get() { return IDose; }
    void Increase(uint32_t Amount, DoIndication_t DoIndication) {
        uint32_t Dz = IDose;
        if(((Dz + Amount) > DOSE_TOP) or (Amount == INFINITY32)) Dz = DOSE_TOP;
        else Dz += Amount;
        Set(Dz, DoIndication);
    }
    void Decrease(uint32_t Amount, DoIndication_t DoIndication) {
        uint32_t Dz = IDose;
        if((Amount > Dz) or (Amount == INFINITY32)) Dz = 0;
        else Dz -= Amount;
        Set(Dz, DoIndication);
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
        Set(FDose, diUsual);
    }
};
static Dose_t Dose;
#endif

#if 1 // ================================ Pill =================================
struct Pill_t {
    uint16_t ID;
    uint16_t Charge;
    uint32_t Value;
} __attribute__ ((__packed__));
static Pill_t Pill;

void App_t::PillHandler() {
    // Read med
    if(PillMgr.Read(PILL_I2C_ADDR, (uint8_t*)&Pill, sizeof(Pill_t)) != OK) return;
    //Uart.Printf("Pill: %u, %X, %u\r", Pill.ID, Pill.Charge, Pill.Value);
    if((Pill.ID == PILL_ID_CURE) and (Pill.Charge != 0)) {
        bool Rslt = OK;
        // Lower charges if not infinity
        if(Pill.Charge != INFINITY16) {
            Pill.Charge--;
            Rslt = PillMgr.Write(PILL_I2C_ADDR, (uint8_t*)&Pill, sizeof(Pill_t));
        }
        if(Rslt == OK) {
            Beeper.Beep(BeepPillOk);
            Led.StartBlink(LedPillOk);
            // Decrease dose if not dead, or if this is panacea
            if((Dose.State != hsDeath) or (Pill.Charge == INFINITY16)) Dose.Decrease(Pill.Value, diNeverIndicate);
            chThdSleep(2007);    // Let indication to complete
            Dose.ChangeIndication();
            return;
        }
    } // if Cure
    // Will be here in case of strange/discharged pill
    Beeper.Beep(BeepPillBad);
    Led.StartBlink(LedPillBad);
    chThdSleep(2007);    // Let indication to complete
    Dose.ChangeIndication();
}

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
    bool PillConnected = false;
    while(true) {
        EvtMsk = chEvtWaitAny(ALL_EVENTS);
        // ==== Process dose ====
        if(EvtMsk & EVTMSK_DOSE_INC) {
            Dose.Increase(1, diUsual);
        }

        // ==== Store dose ====
        if(EvtMsk & EVTMSK_DOSE_STORE) {
            if(Dose.Save() != OK) SignalError("Save Fail\r");
        }

        // ==== Check pill ====
        if(EvtMsk & EVTMSK_PILL_CHECK) {
//            Uart.Printf("c");
            // Check if new connection occured
            if(PillMgr.CheckIfConnected(PILL_I2C_ADDR) == OK) {
                if(!PillConnected) {
                    PillConnected = true;
                    App.PillHandler();
                }
            }
            else PillConnected = false;
        } // if EVTMSK_PILL_CHECK


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
            if(b <= 7) SBuf[1] = PillMgr.CheckIfConnected(PILL_I2C_ADDR);
            SBuf[0] = b;
            Uart.Cmd(RPL_PILL_STATE, SBuf, 2);
            break;
        case CMD_PILL_WRITE:
            b = PData[0];
            if(b <= 7) SBuf[1] = PillMgr.Write(PILL_I2C_ADDR, &PData[1], Length-1);
            SBuf[0] = b;
            Uart.Cmd(RPL_PILL_WRITE, SBuf, 2);
            break;
        case CMD_PILL_READ:
            b = PData[0];           // Pill address
            b2 = PData[1];          // Data size to read
            if(b2 > 250) b2 = 250;  // Check data size
            if(b <= 7) SBuf[1] = PillMgr.Read(PILL_I2C_ADDR, &SBuf[2], b2);
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
            if(w <= DOSE_TOP) {
                Dose.Set(w, diAlwaysIndicate);
                Ack(OK);
            }
            else Ack(FAILURE);
            break;

        default: break;
    } // switch
}
#endif
