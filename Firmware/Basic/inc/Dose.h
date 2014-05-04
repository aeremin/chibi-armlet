/*
 * Dose.h
 *
 *  Created on: 03 мая 2014 г.
 *      Author: Kreyl
 */

#ifndef DOSE_H_
#define DOSE_H_

#include "eestore.h"

struct DoseConsts_t {
    uint32_t Top;       // Death; top value
    uint32_t Red;       // Yellow if lower
    uint32_t Yellow;    // Green if lower
};
#define DOSE_CONSTS_SZ      sizeof(DoseConsts_t)
#define DOSE_RED_FAST_DIF   7  // Near death

#if 1 // ==== Default Dose constants ====
#define DOSE_DEF_TOP        300
#define DOSE_DEF_RED        200
#define DOSE_DEF_YELLOW     100
#endif

enum HealthState_t {hsNone=0, hsGreen, hsYellow, hsRedSlow, hsRedFast, hsDeath};
enum DoIndication_t {diUsual, diAlwaysIndicate, diNeverIndicate};

class Dose_t {
private:
    uint32_t IDose;
    EEStore_t EEStore;   // EEPROM storage for dose
    void ConvertDoseToState() {
        if     (IDose >= Consts.Top)    State = hsDeath;
        else if(IDose >= (Consts.Top - DOSE_RED_FAST_DIF)) State = hsRedFast;
        else if(IDose >= Consts.Red)    State = hsRedSlow;
        else if(IDose >= Consts.Yellow) State = hsYellow;
        else                            State = hsGreen;
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
        uint32_t RedFast = Consts.Top - DOSE_RED_FAST_DIF;
        // Increase no more than up to near death
        if(Dz < RedFast) {
            if(((Dz + Amount) > RedFast) or (Amount == INFINITY32)) Dz = RedFast;
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

#if 1 // ============================ Radio damage =============================
//inline void GetDmgOutOfPkt() {
//    int32_t prc = RSSI_DB2PERCENT(PktRx.RSSI);
////      Uart.Printf("%u\r", PktRx.ID);
//    if(prc >= PktRx.MinLvl) {   // Only if signal level is enough
//        if((PktRx.DmgMax == 0) and (PktRx.DmgMin == 0)) NaturalDmg = 0; // "Clean zone" emanator
//        else {  // Ordinal emanator
//            int32_t EmDmg = 0;
//            if(prc >= PktRx.MaxLvl) EmDmg = PktRx.DmgMax;
//            else {
//                int32_t DifDmg = PktRx.DmgMax - PktRx.DmgMin;
//                int32_t DifLvl = PktRx.MaxLvl - PktRx.MinLvl;
//                EmDmg = (prc * DifDmg + PktRx.DmgMax * DifLvl - PktRx.MaxLvl * DifDmg) / DifLvl;
//                if(EmDmg < 0) EmDmg = 0;
//            }
//            RadioDmg += EmDmg;
//        }
//    }
//}


#endif

#endif /* DOSE_H_ */
