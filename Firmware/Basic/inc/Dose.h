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
    uint32_t Top;       // Death; top value
    uint32_t RedFast;   // Near death
    uint32_t Red;       // Yellow if lower
    uint32_t Yellow;    // Green if lower
    void Setup(uint32_t NewTop) {
        Top = NewTop;
        RedFast = Top - 7;
        Red = (Top * 2) / 3;
        Yellow = Top / 3;
    }
};

// Default Dose constants
#define DOSE_DEFAULT_TOP    300
// Lustra constants
#define LUSTRA_MIN_LVL      650
#define LUSTRA_MAX_LVL      1000
#define LUSTRA_MIN_DMG      1
#define LUSTRA_MAX_DMG      60

enum HealthState_t {hsNone=0, hsGreen, hsYellow, hsRedSlow, hsRedFast, hsDeath};
enum DoIndication_t {diUsual, diAlwaysIndicate, diNeverIndicate};

class Dose_t {
private:
    uint32_t IDose;
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
    void Set(uint32_t ADose, DoIndication_t DoIndication) {
        IDose = ADose;
        HealthState_t OldState = State;
        ConvertDoseToState();
        if(     (DoIndication == diAlwaysIndicate) or
                ((State != OldState) and (DoIndication == diUsual))
                )
            RenewIndication();
    }
    uint32_t Get() { return IDose; }
    void Increase(uint32_t Amount, DoIndication_t DoIndication) {
        uint32_t Dz = IDose;
        // Increase no more than up to near death
        if(Dz < Consts.RedFast) {
            if(((Dz + Amount) > Consts.RedFast) or (Amount == INFINITY32)) Dz = Consts.RedFast;
            else Dz += Amount;
        }
        // Near death, increase no more than 1 at a time
        else if(Dz < Consts.Top) Dz++;
        // After death, no need to increase
//        Uart.Printf("Dz=%u\r", Dz);
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
        if(EEStore.Get(&OldDose) == OK) {
            if(OldDose == IDose) return OK;
        }
        return EEStore.Put(&IDose);
    }
    // Try load from EEPROM, set 0 if failed
    void Load() {
        uint32_t FDose = 0;
        EEStore.Get(&FDose);     // Try to read
        Set(FDose, diUsual);
    }
};

#endif /* DOSE_H_ */
