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
    int32_t IDose;
    EEStore_t EEStore;   // EEPROM storage for dose
    void ConvertDoseToState() {
        if     (IDose >= Consts.Top)     State = hsDeath;
        else if(IDose >= Consts.RedFast) State = hsRedFast;
        else if(IDose >= Consts.Red)     State = hsRedSlow;
        else if(IDose >= Consts.Yellow)  State = hsYellow;
        else                             State = hsGreen;
    }
public:
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
        IDose = ADose;
        HealthState_t OldState = State;
        ConvertDoseToState();
        if(     (DoIndication == diAlwaysIndicate) or
                ((State != OldState) and (DoIndication == diUsual))
                )
            RenewIndication();
    }
    uint32_t Get() { return IDose; }
    void Increase(int32_t Amount, DoIndication_t DoIndication) {
        int32_t Dz = IDose;
        // Increase no more than up to near death
        if(Dz < Consts.RedFast) {
            if(Amount > (Consts.RedFast - Dz)) Dz = Consts.RedFast;
            else Dz += Amount;
        }
        // Near death, increase no more than 1 at a time
        else if(Dz < Consts.Top) Dz++;  // After death, no need to increase
//        Uart.Printf("Dz=%u\r", Dz);
        Set(Dz, DoIndication);
    }
    void Decrease(int32_t Amount, DoIndication_t DoIndication) {
        int32_t Dz = IDose;
        if(Amount > Dz) Dz = 0;
        else Dz -= Amount;
        Set(Dz, DoIndication);
    }
    // Save if changed
    uint8_t Save() {
        int32_t OldDose = 0;
        if(EEStore.Get((uint32_t*)&OldDose) == OK) {
            if(OldDose == IDose) return OK;
        }
        return EEStore.Put((uint32_t*)&IDose);
    }
    // Try load from EEPROM, set 0 if failed
    void Load() {
        uint32_t FDose = 0;
        EEStore.Get(&FDose);     // Try to read
        Set(FDose, diUsual);
    }
};

#endif /* DOSE_H_ */
