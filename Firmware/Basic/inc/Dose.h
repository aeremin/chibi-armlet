/*
 * Dose.h
 *
 *  Created on: 03 мая 2014 г.
 *      Author: Kreyl
 */

#ifndef DOSE_H_
#define DOSE_H_

#include "eestore.h"
#include "rlvl1_defins.h"
#include "peripheral.h"
#include "colors_sounds.h"
#include "cmd_uart.h"

struct DoseConsts_t {
    int32_t Top;       // Death; top value
    int32_t RedFast;   // Near death
    int32_t Red;       // Yellow if lower
    int32_t Yellow;    // Green if lower
    void Setup(int32_t NewTop) {
        Top = NewTop;
        RedFast = Top - 7;
        Red = (Top * 2) / 3;
        Yellow = Top / 3;
    }
};

// Default Dose constants
#define DOSE_DEFAULT_TOP    10800

enum HealthState_t {hsNone=0, hsGreen, hsYellow, hsRedSlow, hsRedFast, hsDeath};
enum DoIndication_t {diUsual, diAlwaysIndicate, diNeverIndicate};

class Dose_t {
private:
    EEStore_t EEStore;   // EEPROM storage for dose
    void ConvertDoseToState() {
        if     (Value >= Consts.Top)     State = hsDeath;
        else if(Value >= Consts.RedFast) State = hsRedFast;
        else if(Value >= Consts.Red)     State = hsRedSlow;
        else if(Value >= Consts.Yellow)  State = hsYellow;
        else                             State = hsGreen;
    }
public:
    int32_t Value;
    DoseConsts_t Consts;
    HealthState_t State;
    void RenewIndication() {
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
    void Set(int32_t ADose, DoIndication_t DoIndication) {
        Value = ADose;
        HealthState_t OldState = State;
        ConvertDoseToState();
        if((DoIndication == diAlwaysIndicate) or
          ((State != OldState) and (DoIndication == diUsual)))
            RenewIndication();
    }
    void Modify(int32_t Amount, DoIndication_t DoIndication) {
        int32_t Dz = Value;
        if(Amount >= 0) { // Doze increase
            // Increase no more than up to near death
            if(Dz < Consts.RedFast) {
                if(Amount > (Consts.RedFast - Dz)) Dz = Consts.RedFast;
                else Dz += Amount;
            }
            // Near death, increase no more than 1 at a time
            else if(Dz < Consts.Top) Dz++;  // After death, no need to increase
        }
        else { // Amount < 0, Dose decrease
            if((- Amount) > Dz) Dz = 0;
            else Dz += Amount;
        }
        Set(Dz, DoIndication);
        Uart.Printf("Dz=%d; Dmg=%d\r", Value, Amount);
    }
    void Reset() { Modify(MIN_INT32, diNeverIndicate); }
    // Save if changed
    uint8_t Save() {
        int32_t OldValue = 0;
        if(EEStore.Get((uint32_t*)&OldValue) == OK) {
            if(OldValue == Value) return OK;
        }
        return EEStore.Put((uint32_t*)&Value);
    }
    // Try load from EEPROM, set 0 if failed
    void Load() {
        int32_t FDose = 0;
        EEStore.Get((uint32_t*)&FDose);     // Try to read
        Set(FDose, diUsual);
    }
};

#endif /* DOSE_H_ */
